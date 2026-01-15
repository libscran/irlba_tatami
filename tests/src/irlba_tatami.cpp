#include <gtest/gtest.h>

#include "irlba_tatami/irlba_tatami.hpp"
#include "Eigen/Dense"

class TatamiTest : public ::testing::TestWithParam<std::tuple<std::pair<int, int>, bool, bool, int> > {
protected:
    Eigen::MatrixXd emat;
    std::shared_ptr<tatami::Matrix<double, int> > matptr;
    unsigned long long seed;
    int NR, NC, nthreads;

    void SetUp() {
        auto param = GetParam();
        auto dims = std::get<0>(param);
        auto row_major = std::get<1>(param);
        auto dense = std::get<2>(param);
        nthreads = std::get<3>(param);

        NR = dims.first;
        NC = dims.second;
        emat.resize(NR, NC);  

        seed = dims.first + dims.second * 10 + row_major + dense * 2 + nthreads * 5; // mock up a seed.
        std::mt19937_64 rng(seed);
        std::normal_distribution<> dist;

        if (dense) {
            for (Eigen::Index r = 0; r < NR; ++r) {
                for (Eigen::Index c = 0; c < NC; ++c) {
                    emat(r, c) = dist(rng);
                }
            }
        } else {
            std::uniform_real_distribution<> unif;
            for (Eigen::Index r = 0; r < NR; ++r) {
                for (Eigen::Index c = 0; c < NC; ++c) {
                    if (unif(rng) < 0.2) {
                        emat(r, c) = dist(rng);
                    } else {
                        emat(r, c) = 0;
                    }
                }
            }
        }

        tatami::ArrayView<double> view(emat.data(), static_cast<std::size_t>(NR) * static_cast<std::size_t>(NC));
        tatami::DenseMatrix<double, int, decltype(view)> matview(NR, NC, std::move(view), false);
        if (dense) {
            matptr = tatami::convert_to_dense<double, int>(matview, row_major, tatami::ConvertToDenseOptions());
        } else {
            matptr = tatami::convert_to_compressed_sparse<double, int>(matview, row_major, tatami::ConvertToCompressedSparseOptions());
        }
    }

protected:
    static Eigen::VectorXd simulate_vector(Eigen::Index len, unsigned long long seed) {
        std::mt19937_64 rng(seed);
        std::normal_distribution<> dist;
        Eigen::VectorXd vec(len);
        for (auto& v : vec) {
            v = dist(rng);
        }
        return vec;
    }

    static void expect_near_equal_vectors(const Eigen::VectorXd& left, const Eigen::VectorXd& right, double tol=1e-8) {
        int n = left.size();
        ASSERT_EQ(n, right.size());

        for (int i = 0; i < n; ++i) {
            const auto L = left[i];
            const auto R = right[i];

            // More-or-less copied from scran_tests::compare_almost_equal,
            // but I didn't want to pull that library in just for one function.
            const double delta = std::abs(L - R);
            const double denom = std::abs(L + R) / 2;
            double threshold = denom * tol;

            constexpr double absolute_tolerance = 1e-15;
            if (threshold < absolute_tolerance) { 
                threshold = absolute_tolerance;
            }

            EXPECT_LT(delta, threshold) << "failed comparison between " << L << " and " << R;
        }
    }

    static void expect_near_equal_columns_sans_sign(const Eigen::MatrixXd& left, const Eigen::MatrixXd& right, double tol=1e-8) {
        ASSERT_EQ(left.cols(), right.cols());

        int ncols = left.cols();
        for (int c = 0; c < ncols; ++c) {
            const Eigen::VectorXd lcol = left.col(c);
            Eigen::VectorXd rcol = right.col(c);

            const auto lsum = lcol.sum();
            const auto rsum = rcol.sum();
            if ((lsum > 0) != (rsum > 0)) {
                rcol *= -1; // adjust for any sign differences.
            }

            expect_near_equal_vectors(lcol, rcol, tol);
        }
    }
};

TEST_P(TatamiTest, Transposed) {
    irlba_tatami::Transposed<Eigen::VectorXd, Eigen::MatrixXd, double, int> wrapped(matptr, nthreads);
    EXPECT_EQ(wrapped.rows(), NC);
    EXPECT_EQ(wrapped.cols(), NR);
    Eigen::MatrixXd tmat = emat.adjoint();

    {
        Eigen::MatrixXd realized;
        auto realizer = wrapped.new_realize_workspace();
        realizer->realize_copy(realized);
        EXPECT_EQ(realized, tmat);
    }

    {
        Eigen::VectorXd rhs = simulate_vector(NR, seed + 2000);
        Eigen::VectorXd prod1(NC);
        auto wrk = wrapped.new_workspace();
        wrk->multiply(rhs, prod1);
        Eigen::VectorXd prod2 = tmat * rhs;
        expect_near_equal_vectors(prod1, prod2);
    }

    {
        Eigen::VectorXd rhs = simulate_vector(NC, seed + 3000);
        Eigen::VectorXd tprod1(NR);
        auto wrk = wrapped.new_adjoint_workspace();
        wrk->multiply(rhs, tprod1);
        Eigen::VectorXd tprod2 = emat * rhs;
        expect_near_equal_vectors(tprod1, tprod2);
    }

    {
        irlba::Options opt;
        opt.convergence_tolerance = 1e-8;
        auto ref = irlba::compute(irlba::SimpleMatrix<Eigen::VectorXd, Eigen::MatrixXd, Eigen::MatrixXd*>(&tmat), 3, opt);
        auto svd = irlba::compute(wrapped, 3, opt);

        expect_near_equal_vectors(ref.D, svd.D);
        expect_near_equal_columns_sans_sign(ref.U, svd.U);
        expect_near_equal_columns_sans_sign(ref.V, svd.V);
    }
}

TEST_P(TatamiTest, Normal) {
    irlba_tatami::Normal<Eigen::VectorXd, Eigen::MatrixXd, double, int> wrapped(matptr, nthreads);
    EXPECT_EQ(wrapped.rows(), NR);
    EXPECT_EQ(wrapped.cols(), NC);

    {
        Eigen::MatrixXd realized;
        auto realizer = wrapped.new_realize_workspace();
        realizer->realize_copy(realized);
        EXPECT_EQ(realized, emat);
    }

    {
        Eigen::VectorXd rhs = simulate_vector(NC, seed + 2000);
        Eigen::VectorXd prod1(NR);
        auto wrk = wrapped.new_workspace();
        wrk->multiply(rhs, prod1);
        Eigen::VectorXd prod2 = emat * rhs;
        expect_near_equal_vectors(prod1, prod2);
    }

    {
        Eigen::VectorXd rhs = simulate_vector(NR, seed + 3000);
        Eigen::VectorXd tprod1(NC);
        auto wrk = wrapped.new_adjoint_workspace();
        wrk->multiply(rhs, tprod1);
        Eigen::VectorXd tprod2 = emat.adjoint() * rhs;
        expect_near_equal_vectors(tprod1, tprod2);
    }

    {
        irlba::Options opt;
        opt.convergence_tolerance = 1e-8;
        auto ref = irlba::compute(irlba::SimpleMatrix<Eigen::VectorXd, Eigen::MatrixXd, Eigen::MatrixXd*>(&emat), 3, opt);
        auto svd = irlba::compute(wrapped, 3, opt);

        expect_near_equal_vectors(ref.D, svd.D);
        expect_near_equal_columns_sans_sign(ref.U, svd.U);
        expect_near_equal_columns_sans_sign(ref.V, svd.V);
    }
}

INSTANTIATE_TEST_SUITE_P(
    IrlbaTatami,
    TatamiTest,
    ::testing::Combine(
        ::testing::Values(
            std::make_pair(20, 80),
            std::make_pair(80, 20),
            std::make_pair(40, 50)
        ),
        ::testing::Values(false, true),
        ::testing::Values(false, true),
        ::testing::Values(1, 3)
    )
);
