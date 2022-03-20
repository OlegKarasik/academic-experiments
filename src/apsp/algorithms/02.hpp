#pragma once

#define APSP_ALG_SETUP

#include "square-shape.hpp"
#include "memory.hpp"

template<typename T, typename A>
__attribute__((noinline)) bool
setup_apsp(::utilz::square_shape<T, A>& m, ::utilz::memory::buff_buf buff_buf)
{
  return true;
}

// #define FMAX 1000000000
// static int mck[N];
// static int drk[N];
// static int mrk[N];
// static int wrk[N];
// apsp::constants::infinity<value_type>()
template<typename T, typename A>
__attribute__((noinline)) void
calculate_apsp(::utilz::square_shape<T, A>& m, bool value)
{
  // using size_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;
  // using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::value_type;

	// int i, j, k;
	// int* pDij;
	// int minR;
	// int sumR, sumC, sumDij;
	// int drki;
	// int* pBik;
	// int* pBki, bki;
	// int* pdck, * pmrk, * pwrk, * pmck;

	// drk[0] = FMAX;
	// wrk[0] = B1[1];
	// for (k = 1; k < N; ++k) {
	// 	pDck = &B1[(k - 1) * N];
	// 	for (i = 0; i < k; ++i)
	// 		mck[i] = FMAX;
	// 	pBki = &B1[k * N];
	// 	pmrk = &mrk[0];
	// 	for (i = 0; i < k; ++i, ++pmrk) {
	// 		minR = FMAX;
	// 		bki = pBki[i];
	// 		drki = drk[i];
	// 		pDij = &B1[i * N];
	// 		pdck = pDck;
	// 		for (j = 0; j < k; ++j, ++pDij, ++pdck) {
	// 			sumDij = drki + *pdck;
	// 			if (*pDij > sumDij) *pDij = sumDij;
	// 			sumR = *pDij + wrk[j];
	// 			if (minR > sumR) minR = sumR;
	// 			sumC = *pDij + bki;
	// 			if (mck[j] > sumC) mck[j] = sumC;
	// 		}
	// 		*pmrk = minR;
	// 	}
	// 	pBki = &B1[k * N];
	// 	pBik = &B1[k];
	// 	pwrk = &wrk[0];
	// 	pmck = &mck[0];
	// 	for (i = 0; i < k; ++i, ++pBki, pBik += N, ++pwrk, ++pmck) {
	// 		*pBki = *pmck;
	// 		*pBik = drk[i] = mrk[i];
	// 		*pwrk = pBik[1];
	// 	}
	// 	if (k < N - 1) *pwrk = pBik[1];
	// }
	// for (i = 0; i < N - 1; ++i) {
	// 	drki = drk[i];
	// 	pDij = &B1[i * N];
	// 	pdck = &B1[(N - 1) * N];
	// 	for (j = 0; j < N - 1; ++j, ++pDij, ++pdck) {
	// 		sumDij = drki + *pdck;
	// 		if (*pDij > sumDij) *pDij = sumDij;
	// 	}
	// }
};
