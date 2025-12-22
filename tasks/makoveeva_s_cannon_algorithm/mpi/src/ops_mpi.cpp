#include "makoveeva_s_cannon_algorithm/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>

#include "makoveeva_s_cannon_algorithm/common/include/common.hpp"

namespace makoveeva_s_cannon_algorithm {

namespace {

bool CheckInputLocal(const std::vector<double> &a, const std::vector<double> &b, int n) {
  if (n <= 0) {
    return false;
  }
  const auto n_sz = static_cast<std::size_t>(n);
  const auto expected = n_sz * n_sz;
  return (a.size() == expected) && (b.size() == expected);
}

int ChooseGridQ(int size, int n) {
  if (size <= 0 || n <= 0) {
    return 0;
  }
  int q = static_cast<int>(std::floor(std::sqrt(static_cast<double>(size))));
  while (q > 0) {
    if ((q * q) <= size && (n % q == 0)) {
      return q;
    }
    --q;
  }
  return 0;
}

MPI_Datatype MakeBlockType(int n, int bs) {
  MPI_Datatype block = MPI_DATATYPE_NULL;
  MPI_Type_vector(bs, bs, n, MPI_DOUBLE, &block);

  MPI_Datatype resized = MPI_DATATYPE_NULL;
  MPI_Type_create_resized(block, 0, static_cast<MPI_Aint>(sizeof(double)), &resized);

  MPI_Type_commit(&resized);
  MPI_Type_free(&block);
  return resized;
}

void LocalMatMulAcc(const std::vector<double> &a, const std::vector<double> &b, int bs, std::vector<double> *c) {
  const auto bs_sz = static_cast<std::size_t>(bs);
  for (int i = 0; i < bs; ++i) {
    const auto i_sz = static_cast<std::size_t>(i);
    for (int k = 0; k < bs; ++k) {
      const auto k_sz = static_cast<std::size_t>(k);
      const double a_ik = a[(i_sz * bs_sz) + k_sz];
      for (int j = 0; j < bs; ++j) {
        const auto j_sz = static_cast<std::size_t>(j);
        (*c)[(i_sz * bs_sz) + j_sz] += a_ik * b[(k_sz * bs_sz) + j_sz];
      }
    }
  }
}

}  // namespace

MakoveevaSCannonAlgorithmMPI::MakoveevaSCannonAlgorithmMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = OutType{};
}

bool MakoveevaSCannonAlgorithmMPI::ValidationImpl() {
  int world_rank = 0;
  int world_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int n = 0;
  if (world_rank == 0) {
    n = std::get<2>(GetInput());
  }
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int ok_input = 0;
  if (world_rank == 0) {
    const auto &a = std::get<0>(GetInput());
    const auto &b = std::get<1>(GetInput());
    ok_input = (GetOutput().empty() && CheckInputLocal(a, b, n)) ? 1 : 0;
  }
  MPI_Bcast(&ok_input, 1, MPI_INT, 0, MPI_COMM_WORLD);

  const int q = ChooseGridQ(world_size, n);
  const int ok_grid = (q > 0) ? 1 : 0;

  return (ok_input != 0) && (ok_grid != 0);
}

bool MakoveevaSCannonAlgorithmMPI::PreProcessingImpl() {
  if (!GetOutput().empty()) {
    GetOutput().clear();
  }
  return true;
}

bool MakoveevaSCannonAlgorithmMPI::RunImpl() {
  int world_rank = 0;
  int world_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int n = 0;
  if (world_rank == 0) {
    n = std::get<2>(GetInput());
  }
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (n <= 0) {
    return false;
  }

  const int q = ChooseGridQ(world_size, n);
  if (q <= 0) {
    return false;
  }

  const int active_p = q * q;
  const bool is_active = (world_rank < active_p);

  MPI_Comm active_comm = MPI_COMM_NULL;
  const int color = is_active ? 0 : MPI_UNDEFINED;
  MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &active_comm);

  std::vector<double> c_full;

  if (is_active) {
    int active_rank = 0;
    int active_size = 0;
    MPI_Comm_rank(active_comm, &active_rank);
    MPI_Comm_size(active_comm, &active_size);

    (void)active_size;

    const int bs = n / q;
    const auto bs_sz = static_cast<std::size_t>(bs);

    const std::array<int, 2> dims = {q, q};
    const std::array<int, 2> periods = {1, 1};

    MPI_Comm cart_comm = MPI_COMM_NULL;
    MPI_Cart_create(active_comm, 2, dims.data(), periods.data(), 0 /*reorder*/, &cart_comm);

    int cart_rank = 0;
    MPI_Comm_rank(cart_comm, &cart_rank);

    std::array<int, 2> coords = {0, 0};
    MPI_Cart_coords(cart_comm, cart_rank, 2, coords.data());
    const int row = coords[0];
    const int col = coords[1];

    std::vector<double> a_block(bs_sz * bs_sz, 0.0);
    std::vector<double> b_block(bs_sz * bs_sz, 0.0);
    std::vector<double> c_block(bs_sz * bs_sz, 0.0);

    MPI_Datatype block_type = MakeBlockType(n, bs);

    std::vector<int> sendcounts;
    std::vector<int> displs;
    if (cart_rank == 0) {
      sendcounts.assign(static_cast<std::size_t>(active_p), 1);
      displs.assign(static_cast<std::size_t>(active_p), 0);

      for (int row_idx = 0; row_idx < q; ++row_idx) {
        for (int col_idx = 0; col_idx < q; ++col_idx) {
          const int proc = (row_idx * q) + col_idx;
          displs[static_cast<std::size_t>(proc)] = ((row_idx * bs) * n) + (col_idx * bs);
        }
      }
    }

    const double *a_full_ptr = nullptr;
    const double *b_full_ptr = nullptr;
    if (world_rank == 0) {
      a_full_ptr = std::get<0>(GetInput()).data();
      b_full_ptr = std::get<1>(GetInput()).data();
    }

    const int *sendcounts_ptr = (cart_rank == 0) ? sendcounts.data() : nullptr;
    const int *displs_ptr = (cart_rank == 0) ? displs.data() : nullptr;

    MPI_Scatterv(a_full_ptr, sendcounts_ptr, displs_ptr, block_type, a_block.data(), bs * bs, MPI_DOUBLE, 0, cart_comm);

    MPI_Scatterv(b_full_ptr, sendcounts_ptr, displs_ptr, block_type, b_block.data(), bs * bs, MPI_DOUBLE, 0, cart_comm);

    int src = 0;
    int dst = 0;

    MPI_Cart_shift(cart_comm, 1, -row, &src, &dst);
    MPI_Sendrecv_replace(a_block.data(), bs * bs, MPI_DOUBLE, dst, 0, src, 0, cart_comm, MPI_STATUS_IGNORE);

    MPI_Cart_shift(cart_comm, 0, -col, &src, &dst);
    MPI_Sendrecv_replace(b_block.data(), bs * bs, MPI_DOUBLE, dst, 1, src, 1, cart_comm, MPI_STATUS_IGNORE);

    for (int step = 0; step < q; ++step) {
      LocalMatMulAcc(a_block, b_block, bs, &c_block);

      if (step + 1 < q) {
        MPI_Cart_shift(cart_comm, 1, -1, &src, &dst);
        MPI_Sendrecv_replace(a_block.data(), bs * bs, MPI_DOUBLE, dst, 2, src, 2, cart_comm, MPI_STATUS_IGNORE);

        MPI_Cart_shift(cart_comm, 0, -1, &src, &dst);
        MPI_Sendrecv_replace(b_block.data(), bs * bs, MPI_DOUBLE, dst, 3, src, 3, cart_comm, MPI_STATUS_IGNORE);
      }
    }

    double *c_recv_ptr = nullptr;
    if (cart_rank == 0) {
      c_full.assign(static_cast<std::size_t>(n) * static_cast<std::size_t>(n), 0.0);
      c_recv_ptr = c_full.data();
    }

    MPI_Gatherv(c_block.data(), bs * bs, MPI_DOUBLE, c_recv_ptr, sendcounts_ptr, displs_ptr, block_type, 0, cart_comm);

    MPI_Type_free(&block_type);
    MPI_Comm_free(&cart_comm);
    MPI_Comm_free(&active_comm);
  }

  int out_size = 0;
  if (world_rank == 0) {
    out_size = static_cast<int>(c_full.size());
  }
  MPI_Bcast(&out_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (world_rank != 0) {
    c_full.assign(static_cast<std::size_t>(out_size), 0.0);
  }
  if (out_size > 0) {
    MPI_Bcast(c_full.data(), out_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  }

  GetOutput() = std::move(c_full);
  return true;
}

bool MakoveevaSCannonAlgorithmMPI::PostProcessingImpl() {
  int world_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  int n = 0;
  if (world_rank == 0) {
    n = std::get<2>(GetInput());
  }
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (n <= 0) {
    return false;
  }
  const auto n_sz = static_cast<std::size_t>(n);
  return GetOutput().size() == (n_sz * n_sz);
}

}  // namespace makoveeva_s_cannon_algorithm
