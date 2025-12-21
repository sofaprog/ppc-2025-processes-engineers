#include "makoveeva_s_cannon_algorithm/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cmath>
#include <cstddef>
#include <tuple>
#include <vector>

#include "makoveeva_s_cannon_algorithm/common/include/common.hpp"

namespace makoveeva_s_cannon_algorithm {

namespace {

bool IsPerfectSquare(int p, int* q) {
  if (p <= 0) return false;
  const double root = std::sqrt(static_cast<double>(p));
  const int r = static_cast<int>(std::lround(root));
  if (r * r != p) return false;
  if (q != nullptr) *q = r;
  return true;
}

bool CheckInputLocal(const std::vector<double>& a, const std::vector<double>& b, int n) {
  if (n <= 0) return false;
  const auto n_sz = static_cast<std::size_t>(n);
  const auto exp = n_sz * n_sz;
  return (a.size() == exp) && (b.size() == exp);
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

void LocalMatMulAcc(const std::vector<double>& a, const std::vector<double>& b, int bs, std::vector<double>* c) {
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

MakoveevaSCannonAlgorithmMPI::MakoveevaSCannonAlgorithmMPI(const InType& in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = OutType{};
}

bool MakoveevaSCannonAlgorithmMPI::ValidationImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int n = 0;
  if (rank == 0) {
    n = std::get<2>(GetInput());
  }
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int q = 0;
  int ok_grid = IsPerfectSquare(size, &q) ? 1 : 0;
  ok_grid = (ok_grid != 0 && q > 0 && (n % q == 0)) ? 1 : 0;

  int ok_input = 0;
  if (rank == 0) {
    const auto& a = std::get<0>(GetInput());
    const auto& b = std::get<1>(GetInput());
    ok_input = (GetOutput().empty() && CheckInputLocal(a, b, n)) ? 1 : 0;
  }

  MPI_Bcast(&ok_input, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&ok_grid, 1, MPI_INT, 0, MPI_COMM_WORLD);

  return (ok_input != 0) && (ok_grid != 0);
}

bool MakoveevaSCannonAlgorithmMPI::PreProcessingImpl() {
  if (!GetOutput().empty()) {
    GetOutput().clear();
  }
  return true;
}

bool MakoveevaSCannonAlgorithmMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int q = 0;
  if (!IsPerfectSquare(size, &q)) return false;

  int n = 0;
  if (rank == 0) n = std::get<2>(GetInput());
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  if (n <= 0 || (n % q != 0)) return false;

  const int bs = n / q;
  const auto bs_sz = static_cast<std::size_t>(bs);

  const int dims[2] = {q, q};
  const int periods[2] = {1, 1};
  MPI_Comm cart_comm = MPI_COMM_NULL;

  MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart_comm);

  int cart_rank = 0;
  MPI_Comm_rank(cart_comm, &cart_rank);

  int coords[2] = {0, 0};
  MPI_Cart_coords(cart_comm, cart_rank, 2, coords);
  const int row = coords[0];
  const int col = coords[1];

  std::vector<double> a_block(bs_sz * bs_sz, 0.0);
  std::vector<double> b_block(bs_sz * bs_sz, 0.0);
  std::vector<double> c_block(bs_sz * bs_sz, 0.0);

  MPI_Datatype block_type = MakeBlockType(n, bs);

  std::vector<int> sendcounts;
  std::vector<int> displs;
  if (cart_rank == 0) {
    sendcounts.assign(static_cast<std::size_t>(size), 1);
    displs.assign(static_cast<std::size_t>(size), 0);

    for (int r = 0; r < q; ++r) {
      for (int c = 0; c < q; ++c) {
        const int proc = r * q + c;
        displs[static_cast<std::size_t>(proc)] = (r * bs) * n + (c * bs);
      }
    }
  }

  const double* a_full = nullptr;
  const double* b_full = nullptr;
  if (cart_rank == 0) {
    a_full = std::get<0>(GetInput()).data();
    b_full = std::get<1>(GetInput()).data();
  }

  MPI_Scatterv(a_full,
               cart_rank == 0 ? sendcounts.data() : nullptr,
               cart_rank == 0 ? displs.data() : nullptr,
               block_type,
               a_block.data(), bs * bs, MPI_DOUBLE,
               0, cart_comm);

  MPI_Scatterv(b_full,
               cart_rank == 0 ? sendcounts.data() : nullptr,
               cart_rank == 0 ? displs.data() : nullptr,
               block_type,
               b_block.data(), bs * bs, MPI_DOUBLE,
               0, cart_comm);

  int src = 0, dst = 0;

  MPI_Cart_shift(cart_comm, 1, -row, &src, &dst);
  MPI_Sendrecv_replace(a_block.data(), bs * bs, MPI_DOUBLE, dst, 0, src, 0, cart_comm, MPI_STATUS_IGNORE);

  MPI_Cart_shift(cart_comm, 0, -col, &src, &dst);
  MPI_Sendrecv_replace(b_block.data(), bs * bs, MPI_DOUBLE, dst, 1, src, 1, cart_comm, MPI_STATUS_IGNORE);

  for (int step = 0; step < q; ++step) {
    LocalMatMulAcc(a_block, b_block, bs, &c_block);

    if (step < q - 1) {
      MPI_Cart_shift(cart_comm, 1, -1, &src, &dst);
      MPI_Sendrecv_replace(a_block.data(), bs * bs, MPI_DOUBLE, dst, 2, src, 2, cart_comm, MPI_STATUS_IGNORE);

      MPI_Cart_shift(cart_comm, 0, -1, &src, &dst);
      MPI_Sendrecv_replace(b_block.data(), bs * bs, MPI_DOUBLE, dst, 3, src, 3, cart_comm, MPI_STATUS_IGNORE);
    }
  }

  std::vector<double> c_full;
  if (cart_rank == 0) {
    c_full.assign(static_cast<std::size_t>(n) * static_cast<std::size_t>(n), 0.0);
  }

  MPI_Gatherv(c_block.data(), bs * bs, MPI_DOUBLE,
              cart_rank == 0 ? c_full.data() : nullptr,
              cart_rank == 0 ? sendcounts.data() : nullptr,
              cart_rank == 0 ? displs.data() : nullptr,
              block_type, 0, cart_comm);

  MPI_Type_free(&block_type);
  MPI_Comm_free(&cart_comm);

  if (rank == 0) {
    GetOutput() = std::move(c_full);
  } else {
    GetOutput().clear();
  }
  
  return true;
}

bool MakoveevaSCannonAlgorithmMPI::PostProcessingImpl() {
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int n = 0;
  if (rank == 0) {
    n = std::get<2>(GetInput());
  }
  
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    return true;
  }

  if (n <= 0) return false;
  const auto n_sz = static_cast<std::size_t>(n);
  return GetOutput().size() == (n_sz * n_sz);
}

}  // namespace makoveeva_s_cannon_algorithm