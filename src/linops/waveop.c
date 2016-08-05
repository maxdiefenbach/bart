/* Copyright 2015. The Regents of the University of California.
 * Copyright 2016. Martin Uecker.
 * All rights reserved. Use of this source code is governed by
 * a BSD-style license which can be found in the LICENSE file.
 *
 * Authors:
 * 2014-2016 Martin Uecker <martin.uecker@med.uni-goettingen.de>
 */

#include <assert.h>
#include <complex.h>

#include "num/multind.h"
#include "num/flpmath.h"

#include "linops/linop.h"

#include "misc/misc.h"

#include "wavelet3/wavelet.h"

#include "waveop.h"

struct wavelet_s {

	linop_data_t base;

	unsigned int N;
	unsigned int flags;
	const long* dims;
	const long* istr;
	const long* minsize;
};

static void wavelet_forward(const linop_data_t* _data, complex float* dst, const complex float* src)
{
	const struct wavelet_s* data = CONTAINER_OF(_data, const struct wavelet_s, base);

	long shifts[data->N];
	for (unsigned int i = 0; i < data->N; i++)
		shifts[i] = 0;

	fwt(data->N, data->flags, shifts, data->dims, dst, data->istr, src, data->minsize, 4, wavelet3_dau2);
}

static void wavelet_adjoint(const linop_data_t* _data, complex float* dst, const complex float* src)
{
	const struct wavelet_s* data = CONTAINER_OF(_data, const struct wavelet_s, base);

	long shifts[data->N];
	for (unsigned int i = 0; i < data->N; i++)
		shifts[i] = 0;

	iwt(data->N, data->flags, shifts, data->dims, data->istr, dst, src, data->minsize, 4, wavelet3_dau2);
}

static void wavelet_del(const linop_data_t* _data)
{
	const struct wavelet_s* data = CONTAINER_OF(_data, const struct wavelet_s, base);

	// free stuff

	free((void*)data);
}

struct linop_s* linop_wavelet3_create(unsigned int N, unsigned int flags, const long dims[N], const long istr[N], const long minsize[N])
{
	PTR_ALLOC(struct wavelet_s, data);

	// FIXME copy stuff;
	data->N = N;
	data->flags = flags;
	data->dims = dims;
	data->istr = istr;
	data->minsize = minsize;

	long odims[N];
	md_singleton_dims(N, odims);
//	md_select_dims(N, ~flags, odims, dims);

	assert(1 == odims[0]);

	odims[0] = wavelet_coeffs(N, flags, dims, minsize, 4);

	long ostr[N];
	md_calc_strides(N, ostr, odims, CFL_SIZE);

	return linop_create2(N, odims, ostr, N, dims, istr, &data->base, wavelet_forward, wavelet_adjoint, NULL, NULL, wavelet_del);
}



