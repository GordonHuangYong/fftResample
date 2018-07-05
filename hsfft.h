#ifndef HSFFT_H_
#define HSFFT_H_

#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PI2 6.28318530717958647692528676655900577f
  
typedef struct {
    float re;
    float im;
} fft_t;

typedef struct fft_set *fft_object;

fft_object fft_init(int N, int sgn);

struct fft_set {
    int N;
    int sgn;
    int factors[64];
    int lf;
    int lt;
    fft_t twiddle[1];
};

void fft_exec(fft_object obj, fft_t *inp, fft_t *oup);

int divideby(int M, int d);

int dividebyN(int N);

//void arrrev(int M, int* arr);

int factors(int M, int *arr);

void twiddle(fft_t *sig, int N, int radix);

void longvectorN(fft_t *sig, int N, int *array, int M);

void free_fft(fft_object object);

#ifdef __cplusplus
}
#endif
#ifndef REAL_H_
#define REAL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fft_real_set *fft_real_object;

fft_real_object fft_real_init(int N, int sgn);

struct fft_real_set {
    fft_object cobj;
    fft_t twiddle2[1];
};

void fft_r2c_exec(fft_real_object obj, float *inp, fft_t *oup);

void fft_c2r_exec(fft_real_object obj, fft_t *inp, float *oup);

void free_real_fft(fft_real_object object);

#ifdef __cplusplus
}
#endif

#endif /* REAL_H_ */


#endif /* HSFFT_H_ */

fft_real_object fft_real_init(int N, int sgn) {
    fft_real_object obj = NULL;
    float PI, theta;
    int k;

    PI = 3.1415926535897932384626433832795f;

    obj = (fft_real_object) malloc(sizeof(struct fft_real_set) + sizeof(fft_t) * (N / 2));

    obj->cobj = fft_init(N / 2, sgn);

    for (k = 0; k < N / 2; ++k) {
        theta = PI2 * k / N;
        obj->twiddle2[k].re = cosf(theta);
        obj->twiddle2[k].im = sinf(theta);

    }


    return obj;


}

void fft_r2c_exec(fft_real_object obj, float *inp, fft_t *oup) {
    fft_t *cinp;
    fft_t *coup;
    int i, N2, N;
    float temp1, temp2;
    N2 = obj->cobj->N;
    N = N2 * 2;

    cinp = (fft_t *) malloc(sizeof(fft_t) * N2);
    coup = (fft_t *) malloc(sizeof(fft_t) * N2);

    for (i = 0; i < N2; ++i) {
        cinp[i].re = inp[2 * i];
        cinp[i].im = inp[2 * i + 1];
    }

    fft_exec(obj->cobj, cinp, coup);

    oup[0].re = coup[0].re + coup[0].im;
    oup[0].im = 0.0;

    for (i = 1; i < N2; ++i) {
        temp1 = coup[i].im + coup[N2 - i].im;
        temp2 = coup[N2 - i].re - coup[i].re;
        oup[i].re =
                (coup[i].re + coup[N2 - i].re + (temp1 * obj->twiddle2[i].re) + (temp2 * obj->twiddle2[i].im)) / 2.0f;
        oup[i].im =
                (coup[i].im - coup[N2 - i].im + (temp2 * obj->twiddle2[i].re) - (temp1 * obj->twiddle2[i].im)) / 2.0f;
    }


    oup[N2].re = coup[0].re - coup[0].im;
    oup[N2].im = 0.0;

    for (i = 1; i < N2; ++i) {
        oup[N - i].re = oup[i].re;
        oup[N - i].im = -oup[i].im;
    }


    free(cinp);
    free(coup);

}

void fft_c2r_exec(fft_real_object obj, fft_t *inp, float *oup) {

    fft_t *cinp;
    fft_t *coup;
    int i, N2, N;
    float temp1, temp2;
    N2 = obj->cobj->N;
    N = N2 * 2;

    cinp = (fft_t *) malloc(sizeof(fft_t) * N2);
    coup = (fft_t *) malloc(sizeof(fft_t) * N2);

    for (i = 0; i < N2; ++i) {
        temp1 = -inp[i].im - inp[N2 - i].im;
        temp2 = -inp[N2 - i].re + inp[i].re;
        cinp[i].re = inp[i].re + inp[N2 - i].re + (temp1 * obj->twiddle2[i].re) - (temp2 * obj->twiddle2[i].im);
        cinp[i].im = inp[i].im - inp[N2 - i].im + (temp2 * obj->twiddle2[i].re) + (temp1 * obj->twiddle2[i].im);
    }

    fft_exec(obj->cobj, cinp, coup);
    for (i = 0; i < N2; ++i) {
        oup[2 * i] = coup[i].re;
        oup[2 * i + 1] = coup[i].im;
    }
    free(cinp);
    free(coup);


}

void free_real_fft(fft_real_object object) {
    free_fft(object->cobj);
    free(object);
}

fft_object fft_init(int N, int sgn) {
    fft_object obj = NULL;

    int twi_len, ct, out;
    out = dividebyN(N);

    if (out == 1) {
        obj = (fft_object) malloc(sizeof(struct fft_set) + sizeof(fft_t) * (N - 1));
        obj->lf = factors(N, obj->factors);
        longvectorN(obj->twiddle, N, obj->factors, obj->lf);
        twi_len = N;
        obj->lt = 0;
    } else {
        int K, M;
        K = (int) pow(2.0, ceil(log10(N) / log10(2.0)));

        if (K < 2 * N - 2) {
            M = K * 2;
        } else {
            M = K;
        }
        obj = (fft_object) malloc(sizeof(struct fft_set) + sizeof(fft_t) * (M - 1));
        obj->lf = factors(M, obj->factors);
        longvectorN(obj->twiddle, M, obj->factors, obj->lf);
        obj->lt = 1;
        twi_len = M;
    }


    obj->N = N;
    obj->sgn = sgn;

    if (sgn == -1) {
        for (ct = 0; ct < twi_len; ct++) {
            (obj->twiddle + ct)->im = -(obj->twiddle + ct)->im;

        }
    }

    return obj;
}


static void mixed_radix_dit_rec(fft_t *op, fft_t *ip, const fft_object obj, int sgn, int N, int l, int inc) {
    int radix = 0, m = 0, ll = 0;
    if (N > 1) {
        radix = obj->factors[inc];
    }

    if (N == 1) {

        op[0].re = ip[0].re;
        op[0].im = ip[0].im;

    } else if (N == 2) {
        float tau1r, tau1i;
        op[0].re = ip[0].re;
        op[0].im = ip[0].im;

        op[1].re = ip[l].re;
        op[1].im = ip[l].im;

        tau1r = op[0].re;
        tau1i = op[0].im;


        op[0].re = tau1r + op[1].re;
        op[0].im = tau1i + op[1].im;

        op[1].re = tau1r - op[1].re;
        op[1].im = tau1i - op[1].im;

    } else if (N == 3) {
        float tau0r, tau0i, tau1r, tau1i, tau2r, tau2i;
        op[0].re = ip[0].re;
        op[0].im = ip[0].im;

        op[1].re = ip[l].re;
        op[1].im = ip[l].im;

        op[2].re = ip[2 * l].re;
        op[2].im = ip[2 * l].im;

        tau0r = op[1].re + op[2].re;
        tau0i = op[1].im + op[2].im;

        tau1r = sgn * 0.86602540378f * (op[1].re - op[2].re);
        tau1i = sgn * 0.86602540378f * (op[1].im - op[2].im);

        tau2r = op[0].re - tau0r * 0.5000000000f;
        tau2i = op[0].im - tau0i * 0.5000000000f;

        op[0].re = tau0r + op[0].re;
        op[0].im = tau0i + op[0].im;

        op[1].re = tau2r + tau1i;
        op[1].im = tau2i - tau1r;

        op[2].re = tau2r - tau1i;
        op[2].im = tau2i + tau1r;

        return;


    } else if (N == 4) {
        float tau0r, tau0i, tau1r, tau1i, tau2r, tau2i, tau3r, tau3i;
        op[0].re = ip[0].re;
        op[0].im = ip[0].im;

        op[1].re = ip[l].re;
        op[1].im = ip[l].im;

        op[2].re = ip[2 * l].re;
        op[2].im = ip[2 * l].im;

        op[3].re = ip[3 * l].re;
        op[3].im = ip[3 * l].im;

        tau0r = op[0].re + op[2].re;
        tau0i = op[0].im + op[2].im;

        tau1r = op[0].re - op[2].re;
        tau1i = op[0].im - op[2].im;

        tau2r = op[1].re + op[3].re;
        tau2i = op[1].im + op[3].im;

        tau3r = sgn * (op[1].re - op[3].re);
        tau3i = sgn * (op[1].im - op[3].im);

        op[0].re = tau0r + tau2r;
        op[0].im = tau0i + tau2i;

        op[1].re = tau1r + tau3i;
        op[1].im = tau1i - tau3r;

        op[2].re = tau0r - tau2r;
        op[2].im = tau0i - tau2i;

        op[3].re = tau1r - tau3i;
        op[3].im = tau1i + tau3r;


    } else if (N == 5) {
        float tau0r, tau0i, tau1r, tau1i, tau2r, tau2i, tau3r, tau3i, tau4r, tau4i, tau5r, tau5i, tau6r, tau6i;
        float c1, c2, s1, s2;
        op[0].re = ip[0].re;
        op[0].im = ip[0].im;

        op[1].re = ip[l].re;
        op[1].im = ip[l].im;

        op[2].re = ip[2 * l].re;
        op[2].im = ip[2 * l].im;

        op[3].re = ip[3 * l].re;
        op[3].im = ip[3 * l].im;

        op[4].re = ip[4 * l].re;
        op[4].im = ip[4 * l].im;

        c1 = 0.30901699437f;
        c2 = -0.80901699437f;
        s1 = 0.95105651629f;
        s2 = 0.58778525229f;

        tau0r = op[1].re + op[4].re;
        tau2r = op[1].re - op[4].re;
        tau0i = op[1].im + op[4].im;
        tau2i = op[1].im - op[4].im;

        tau1r = op[2].re + op[3].re;
        tau3r = op[2].re - op[3].re;
        tau1i = op[2].im + op[3].im;
        tau3i = op[2].im - op[3].im;


        tau4r = c1 * tau0r + c2 * tau1r;
        tau4i = c1 * tau0i + c2 * tau1i;


        if (sgn == 1) {
            tau5r = s1 * tau2r + s2 * tau3r;
            tau5i = s1 * tau2i + s2 * tau3i;

        } else {
            tau5r = -s1 * tau2r - s2 * tau3r;
            tau5i = -s1 * tau2i - s2 * tau3i;

        }

        tau6r = op[0].re + tau4r;
        tau6i = op[0].im + tau4i;

        op[1].re = tau6r + tau5i;
        op[1].im = tau6i - tau5r;

        op[4].re = tau6r - tau5i;
        op[4].im = tau6i + tau5r;

        tau4r = c2 * tau0r + c1 * tau1r;
        tau4i = c2 * tau0i + c1 * tau1i;


        if (sgn == 1) {
            tau5r = s2 * tau2r - s1 * tau3r;
            tau5i = s2 * tau2i - s1 * tau3i;

        } else {
            tau5r = -s2 * tau2r + s1 * tau3r;
            tau5i = -s2 * tau2i + s1 * tau3i;

        }

        tau6r = op[0].re + tau4r;
        tau6i = op[0].im + tau4i;

        op[2].re = tau6r + tau5i;
        op[2].im = tau6i - tau5r;

        op[3].re = tau6r - tau5i;
        op[3].im = tau6i + tau5r;


        op[0].re += tau0r + tau1r;
        op[0].im += tau0i + tau1i;


    } else if (N == 7) {
        float tau0r, tau0i, tau1r, tau1i, tau2r, tau2i, tau3r, tau3i, tau4r, tau4i, tau5r, tau5i, tau6r, tau6i, tau7r, tau7i;
        float c1, c2, c3, s1, s2, s3;
        op[0].re = ip[0].re;
        op[0].im = ip[0].im;

        op[1].re = ip[l].re;
        op[1].im = ip[l].im;

        op[2].re = ip[2 * l].re;
        op[2].im = ip[2 * l].im;

        op[3].re = ip[3 * l].re;
        op[3].im = ip[3 * l].im;

        op[4].re = ip[4 * l].re;
        op[4].im = ip[4 * l].im;

        op[5].re = ip[5 * l].re;
        op[5].im = ip[5 * l].im;

        op[6].re = ip[6 * l].re;
        op[6].im = ip[6 * l].im;

        c1 = 0.62348980185f;
        c2 = -0.22252093395f;
        c3 = -0.9009688679f;
        s1 = 0.78183148246f;
        s2 = 0.97492791218f;
        s3 = 0.43388373911f;

        tau0r = op[1].re + op[6].re;
        tau3r = op[1].re - op[6].re;

        tau0i = op[1].im + op[6].im;
        tau3i = op[1].im - op[6].im;

        tau1r = op[2].re + op[5].re;
        tau4r = op[2].re - op[5].re;

        tau1i = op[2].im + op[5].im;
        tau4i = op[2].im - op[5].im;

        tau2r = op[3].re + op[4].re;
        tau5r = op[3].re - op[4].re;

        tau2i = op[3].im + op[4].im;
        tau5i = op[3].im - op[4].im;


        tau6r = op[0].re + c1 * tau0r + c2 * tau1r + c3 * tau2r;
        tau6i = op[0].im + c1 * tau0i + c2 * tau1i + c3 * tau2i;

        if (sgn == 1) {
            tau7r = -s1 * tau3r - s2 * tau4r - s3 * tau5r;
            tau7i = -s1 * tau3i - s2 * tau4i - s3 * tau5i;

        } else {
            tau7r = s1 * tau3r + s2 * tau4r + s3 * tau5r;
            tau7i = s1 * tau3i + s2 * tau4i + s3 * tau5i;

        }


        op[1].re = tau6r - tau7i;
        op[6].re = tau6r + tau7i;

        op[1].im = tau6i + tau7r;
        op[6].im = tau6i - tau7r;

        tau6r = op[0].re + c2 * tau0r + c3 * tau1r + c1 * tau2r;
        tau6i = op[0].im + c2 * tau0i + c3 * tau1i + c1 * tau2i;

        if (sgn == 1) {
            tau7r = -s2 * tau3r + s3 * tau4r + s1 * tau5r;
            tau7i = -s2 * tau3i + s3 * tau4i + s1 * tau5i;
        } else {
            tau7r = s2 * tau3r - s3 * tau4r - s1 * tau5r;
            tau7i = s2 * tau3i - s3 * tau4i - s1 * tau5i;

        }


        op[2].re = tau6r - tau7i;
        op[5].re = tau6r + tau7i;
        op[2].im = tau6i + tau7r;
        op[5].im = tau6i - tau7r;

        tau6r = op[0].re + c3 * tau0r + c1 * tau1r + c2 * tau2r;
        tau6i = op[0].im + c3 * tau0i + c1 * tau1i + c2 * tau2i;

        if (sgn == 1) {
            tau7r = -s3 * tau3r + s1 * tau4r - s2 * tau5r;
            tau7i = -s3 * tau3i + s1 * tau4i - s2 * tau5i;

        } else {
            tau7r = s3 * tau3r - s1 * tau4r + s2 * tau5r;
            tau7i = s3 * tau3i - s1 * tau4i + s2 * tau5i;
        }
        op[3].re = tau6r - tau7i;
        op[4].re = tau6r + tau7i;
        op[3].im = tau6i + tau7r;
        op[4].im = tau6i - tau7r;

        op[0].re += tau0r + tau1r + tau2r;
        op[0].im += tau0i + tau1i + tau2i;


    } else if (N == 8) {
        float tau0r, tau0i, tau1r, tau1i, tau2r, tau2i, tau3r, tau3i, tau4r, tau4i, tau5r, tau5i, tau6r, tau6i, tau7r, tau7i, tau8r, tau8i, tau9r, tau9i;
        float c1, s1, temp1r, temp1i, temp2r, temp2i;
        op[0].re = ip[0].re;
        op[0].im = ip[0].im;

        op[1].re = ip[l].re;
        op[1].im = ip[l].im;

        op[2].re = ip[2 * l].re;
        op[2].im = ip[2 * l].im;

        op[3].re = ip[3 * l].re;
        op[3].im = ip[3 * l].im;

        op[4].re = ip[4 * l].re;
        op[4].im = ip[4 * l].im;

        op[5].re = ip[5 * l].re;
        op[5].im = ip[5 * l].im;

        op[6].re = ip[6 * l].re;
        op[6].im = ip[6 * l].im;

        op[7].re = ip[7 * l].re;
        op[7].im = ip[7 * l].im;

        c1 = 0.70710678118654752440084436210485f;
        s1 = 0.70710678118654752440084436210485f;

        tau0r = op[0].re + op[4].re;
        tau4r = op[0].re - op[4].re;

        tau0i = op[0].im + op[4].im;
        tau4i = op[0].im - op[4].im;

        tau1r = op[1].re + op[7].re;
        tau5r = op[1].re - op[7].re;

        tau1i = op[1].im + op[7].im;
        tau5i = op[1].im - op[7].im;

        tau2r = op[3].re + op[5].re;
        tau6r = op[3].re - op[5].re;

        tau2i = op[3].im + op[5].im;
        tau6i = op[3].im - op[5].im;

        tau3r = op[2].re + op[6].re;
        tau7r = op[2].re - op[6].re;

        tau3i = op[2].im + op[6].im;
        tau7i = op[2].im - op[6].im;

        op[0].re = tau0r + tau1r + tau2r + tau3r;
        op[0].im = tau0i + tau1i + tau2i + tau3i;

        op[4].re = tau0r - tau1r - tau2r + tau3r;
        op[4].im = tau0i - tau1i - tau2i + tau3i;

        temp1r = tau1r - tau2r;
        temp1i = tau1i - tau2i;

        temp2r = tau5r + tau6r;
        temp2i = tau5i + tau6i;

        tau8r = tau4r + c1 * temp1r;
        tau8i = tau4i + c1 * temp1i;

        if (sgn == 1) {
            tau9r = -s1 * temp2r - tau7r;
            tau9i = -s1 * temp2i - tau7i;

        } else {
            tau9r = s1 * temp2r + tau7r;
            tau9i = s1 * temp2i + tau7i;

        }


        op[1].re = tau8r - tau9i;
        op[1].im = tau8i + tau9r;

        op[7].re = tau8r + tau9i;
        op[7].im = tau8i - tau9r;

        tau8r = tau0r - tau3r;
        tau8i = tau0i - tau3i;

        if (sgn == 1) {
            tau9r = -tau5r + tau6r;
            tau9i = -tau5i + tau6i;

        } else {
            tau9r = tau5r - tau6r;
            tau9i = tau5i - tau6i;

        }


        op[2].re = tau8r - tau9i;
        op[2].im = tau8i + tau9r;

        op[6].re = tau8r + tau9i;
        op[6].im = tau8i - tau9r;

        tau8r = tau4r - c1 * temp1r;
        tau8i = tau4i - c1 * temp1i;

        if (sgn == 1) {
            tau9r = -s1 * temp2r + tau7r;
            tau9i = -s1 * temp2i + tau7i;

        } else {
            tau9r = s1 * temp2r - tau7r;
            tau9i = s1 * temp2i - tau7i;

        }


        op[3].re = tau8r - tau9i;
        op[3].im = tau8i + tau9r;

        op[5].re = tau8r + tau9i;
        op[5].im = tau8i - tau9r;


    } else if (radix == 2) {
        int k, tkm1, ind;
        float wlr, wli;
        float tau1r, tau1i, tau2r, tau2i;
        m = N / 2;
        ll = 2 * l;
        mixed_radix_dit_rec(op, ip, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + m, ip + l, obj, sgn, m, ll, inc + 1);

        for (k = 0; k < m; k++) {
            ind = m - 1 + k;
            wlr = (obj->twiddle + ind)->re;
            wli = (obj->twiddle + ind)->im;

            tkm1 = k + m;

            tau1r = op[k].re;
            tau1i = op[k].im;

            tau2r = op[tkm1].re * wlr - op[tkm1].im * wli;
            tau2i = op[tkm1].im * wlr + op[tkm1].re * wli;

            op[k].re = tau1r + tau2r;
            op[k].im = tau1i + tau2i;

            op[tkm1].re = tau1r - tau2r;
            op[tkm1].im = tau1i - tau2i;


        }

    } else if (radix == 3) {
        int k, tkm1, tkm2, ind;
        float wlr, wli, wl2r, wl2i;
        float tau0r, tau0i, tau1r, tau1i, tau2r, tau2i;
        float ar, ai, br, bi, cr, ci;
        m = N / 3;
        ll = 3 * l;
        mixed_radix_dit_rec(op, ip, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + m, ip + l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 2 * m, ip + 2 * l, obj, sgn, m, ll, inc + 1);

        for (k = 0; k < m; ++k) {
            ind = m - 1 + 2 * k;
            wlr = (obj->twiddle + ind)->re;
            wli = (obj->twiddle + ind)->im;
            ind++;
            wl2r = (obj->twiddle + ind)->re;
            wl2i = (obj->twiddle + ind)->im;
            tkm1 = k + m;
            tkm2 = tkm1 + m;

            ar = op[k].re;
            ai = op[k].im;

            br = op[tkm1].re * wlr - op[tkm1].im * wli;
            bi = op[tkm1].im * wlr + op[tkm1].re * wli;

            cr = op[tkm2].re * wl2r - op[tkm2].im * wl2i;
            ci = op[tkm2].im * wl2r + op[tkm2].re * wl2i;

            tau0r = br + cr;
            tau0i = bi + ci;

            tau1r = sgn * 0.86602540378f * (br - cr);
            tau1i = sgn * 0.86602540378f * (bi - ci);

            tau2r = ar - tau0r * 0.5000000000f;
            tau2i = ai - tau0i * 0.5000000000f;


            op[k].re = ar + tau0r;
            op[k].im = ai + tau0i;

            op[tkm1].re = tau2r + tau1i;
            op[tkm1].im = tau2i - tau1r;

            op[tkm2].re = tau2r - tau1i;
            op[tkm2].im = tau2i + tau1r;

        }

    } else if (radix == 4) {
        int k, tkm1, tkm2, tkm3, ind;
        float wlr, wli, wl2r, wl2i, wl3r, wl3i;
        float tau0r, tau0i, tau1r, tau1i, tau2r, tau2i, tau3r, tau3i;
        float ar, ai, br, bi, cr, ci, dr, di;
        m = N / 4;
        ll = 4 * l;
        mixed_radix_dit_rec(op, ip, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + m, ip + l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 2 * m, ip + 2 * l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 3 * m, ip + 3 * l, obj, sgn, m, ll, inc + 1);

        tkm1 = m;
        tkm2 = tkm1 + m;
        tkm3 = tkm2 + m;

        ar = op[0].re;
        ai = op[0].im;

        br = op[tkm1].re;
        bi = op[tkm1].im;

        cr = op[tkm2].re;
        ci = op[tkm2].im;

        dr = op[tkm3].re;
        di = op[tkm3].im;

        tau0r = ar + cr;
        tau0i = ai + ci;

        tau1r = ar - cr;
        tau1i = ai - ci;

        tau2r = br + dr;
        tau2i = bi + di;

        tau3r = sgn * (br - dr);
        tau3i = sgn * (bi - di);

        op[0].re = tau0r + tau2r;
        op[0].im = tau0i + tau2i;

        op[tkm1].re = tau1r + tau3i;
        op[tkm1].im = tau1i - tau3r;

        op[tkm2].re = tau0r - tau2r;
        op[tkm2].im = tau0i - tau2i;

        op[tkm3].re = tau1r - tau3i;
        op[tkm3].im = tau1i + tau3r;


        for (k = 1; k < m; k++) {
            ind = m - 1 + 3 * k;
            wlr = (obj->twiddle + ind)->re;
            wli = (obj->twiddle + ind)->im;
            ind++;
            wl2r = (obj->twiddle + ind)->re;
            wl2i = (obj->twiddle + ind)->im;
            ind++;
            wl3r = (obj->twiddle + ind)->re;
            wl3i = (obj->twiddle + ind)->im;

            tkm1 = k + m;
            tkm2 = tkm1 + m;
            tkm3 = tkm2 + m;

            ar = op[k].re;
            ai = op[k].im;

            br = op[tkm1].re * wlr - op[tkm1].im * wli;
            bi = op[tkm1].im * wlr + op[tkm1].re * wli;

            cr = op[tkm2].re * wl2r - op[tkm2].im * wl2i;
            ci = op[tkm2].im * wl2r + op[tkm2].re * wl2i;

            dr = op[tkm3].re * wl3r - op[tkm3].im * wl3i;
            di = op[tkm3].im * wl3r + op[tkm3].re * wl3i;

            tau0r = ar + cr;
            tau0i = ai + ci;

            tau1r = ar - cr;
            tau1i = ai - ci;

            tau2r = br + dr;
            tau2i = bi + di;

            tau3r = sgn * (br - dr);
            tau3i = sgn * (bi - di);

            op[k].re = tau0r + tau2r;
            op[k].im = tau0i + tau2i;

            op[tkm1].re = tau1r + tau3i;
            op[tkm1].im = tau1i - tau3r;

            op[tkm2].re = tau0r - tau2r;
            op[tkm2].im = tau0i - tau2i;

            op[tkm3].re = tau1r - tau3i;
            op[tkm3].im = tau1i + tau3r;

        }

    } else if (radix == 5) {
        int k, tkm1, tkm2, tkm3, tkm4, ind;
        float wlr, wli, wl2r, wl2i, wl3r, wl3i, wl4r, wl4i;
        float tau0r, tau0i, tau1r, tau1i, tau2r, tau2i, tau3r, tau3i;
        float ar, ai, br, bi, cr, ci, dr, di, er, ei;
        float tau4r, tau4i, tau5r, tau5i, tau6r, tau6i;
        float c1, c2, s1, s2;
        m = N / 5;
        ll = 5 * l;
        mixed_radix_dit_rec(op, ip, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + m, ip + l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 2 * m, ip + 2 * l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 3 * m, ip + 3 * l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 4 * m, ip + 4 * l, obj, sgn, m, ll, inc + 1);

        c1 = 0.30901699437f;
        c2 = -0.80901699437f;
        s1 = 0.95105651629f;
        s2 = 0.58778525229f;

        tkm1 = m;
        tkm2 = tkm1 + m;
        tkm3 = tkm2 + m;
        tkm4 = tkm3 + m;

        ar = op[0].re;
        ai = op[0].im;

        br = op[tkm1].re;
        bi = op[tkm1].im;

        cr = op[tkm2].re;
        ci = op[tkm2].im;

        dr = op[tkm3].re;
        di = op[tkm3].im;

        er = op[tkm4].re;
        ei = op[tkm4].im;

        tau0r = br + er;
        tau0i = bi + ei;

        tau1r = cr + dr;
        tau1i = ci + di;

        tau2r = br - er;
        tau2i = bi - ei;

        tau3r = cr - dr;
        tau3i = ci - di;

        op[0].re = ar + tau0r + tau1r;
        op[0].im = ai + tau0i + tau1i;

        tau4r = c1 * tau0r + c2 * tau1r;
        tau4i = c1 * tau0i + c2 * tau1i;

        tau5r = sgn * (s1 * tau2r + s2 * tau3r);
        tau5i = sgn * (s1 * tau2i + s2 * tau3i);

        tau6r = ar + tau4r;
        tau6i = ai + tau4i;

        op[tkm1].re = tau6r + tau5i;
        op[tkm1].im = tau6i - tau5r;

        op[tkm4].re = tau6r - tau5i;
        op[tkm4].im = tau6i + tau5r;

        tau4r = c2 * tau0r + c1 * tau1r;
        tau4i = c2 * tau0i + c1 * tau1i;

        tau5r = sgn * (s2 * tau2r - s1 * tau3r);
        tau5i = sgn * (s2 * tau2i - s1 * tau3i);

        tau6r = ar + tau4r;
        tau6i = ai + tau4i;

        op[tkm2].re = tau6r + tau5i;
        op[tkm2].im = tau6i - tau5r;

        op[tkm3].re = tau6r - tau5i;
        op[tkm3].im = tau6i + tau5r;

        for (k = 1; k < m; k++) {
            ind = m - 1 + 4 * k;
            wlr = (obj->twiddle + ind)->re;
            wli = (obj->twiddle + ind)->im;
            ind++;
            wl2r = (obj->twiddle + ind)->re;
            wl2i = (obj->twiddle + ind)->im;
            ind++;
            wl3r = (obj->twiddle + ind)->re;
            wl3i = (obj->twiddle + ind)->im;
            ind++;
            wl4r = (obj->twiddle + ind)->re;
            wl4i = (obj->twiddle + ind)->im;

            tkm1 = k + m;
            tkm2 = tkm1 + m;
            tkm3 = tkm2 + m;
            tkm4 = tkm3 + m;

            ar = op[k].re;
            ai = op[k].im;

            br = op[tkm1].re * wlr - op[tkm1].im * wli;
            bi = op[tkm1].im * wlr + op[tkm1].re * wli;

            cr = op[tkm2].re * wl2r - op[tkm2].im * wl2i;
            ci = op[tkm2].im * wl2r + op[tkm2].re * wl2i;

            dr = op[tkm3].re * wl3r - op[tkm3].im * wl3i;
            di = op[tkm3].im * wl3r + op[tkm3].re * wl3i;

            er = op[tkm4].re * wl4r - op[tkm4].im * wl4i;
            ei = op[tkm4].im * wl4r + op[tkm4].re * wl4i;

            tau0r = br + er;
            tau0i = bi + ei;

            tau1r = cr + dr;
            tau1i = ci + di;

            tau2r = br - er;
            tau2i = bi - ei;

            tau3r = cr - dr;
            tau3i = ci - di;

            op[k].re = ar + tau0r + tau1r;
            op[k].im = ai + tau0i + tau1i;

            tau4r = c1 * tau0r + c2 * tau1r;
            tau4i = c1 * tau0i + c2 * tau1i;

            if (sgn == 1) {
                tau5r = s1 * tau2r + s2 * tau3r;
                tau5i = s1 * tau2i + s2 * tau3i;

            } else {
                tau5r = -s1 * tau2r - s2 * tau3r;
                tau5i = -s1 * tau2i - s2 * tau3i;

            }

            tau6r = ar + tau4r;
            tau6i = ai + tau4i;

            op[tkm1].re = tau6r + tau5i;
            op[tkm1].im = tau6i - tau5r;

            op[tkm4].re = tau6r - tau5i;
            op[tkm4].im = tau6i + tau5r;

            tau4r = c2 * tau0r + c1 * tau1r;
            tau4i = c2 * tau0i + c1 * tau1i;


            if (sgn == 1) {
                tau5r = s2 * tau2r - s1 * tau3r;
                tau5i = s2 * tau2i - s1 * tau3i;

            } else {
                tau5r = -s2 * tau2r + s1 * tau3r;
                tau5i = -s2 * tau2i + s1 * tau3i;

            }

            tau6r = ar + tau4r;
            tau6i = ai + tau4i;

            op[tkm2].re = tau6r + tau5i;
            op[tkm2].im = tau6i - tau5r;

            op[tkm3].re = tau6r - tau5i;
            op[tkm3].im = tau6i + tau5r;

        }

    } else if (radix == 7) {
        int k, tkm1, tkm2, tkm3, tkm4, tkm5, tkm6, ind;
        float wlr, wli, wl2r, wl2i, wl3r, wl3i, wl4r, wl4i, wl5r, wl5i, wl6r, wl6i;
        float tau0r, tau0i, tau1r, tau1i, tau2r, tau2i, tau3r, tau3i;
        float ar, ai, br, bi, cr, ci, dr, di, er, ei, fr, fi, gr, gi;
        float tau4r, tau4i, tau5r, tau5i, tau6r, tau6i, tau7r, tau7i;
        float c1, c2, c3, s1, s2, s3;
        m = N / 7;
        ll = 7 * l;
        mixed_radix_dit_rec(op, ip, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + m, ip + l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 2 * m, ip + 2 * l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 3 * m, ip + 3 * l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 4 * m, ip + 4 * l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 5 * m, ip + 5 * l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 6 * m, ip + 6 * l, obj, sgn, m, ll, inc + 1);

        c1 = 0.62348980185f;
        c2 = -0.22252093395f;
        c3 = -0.9009688679f;
        s1 = 0.78183148246f;
        s2 = 0.97492791218f;
        s3 = 0.43388373911f;

        tkm1 = m;
        tkm2 = tkm1 + m;
        tkm3 = tkm2 + m;
        tkm4 = tkm3 + m;
        tkm5 = tkm4 + m;
        tkm6 = tkm5 + m;

        ar = op[0].re;
        ai = op[0].im;

        br = op[tkm1].re;
        bi = op[tkm1].im;

        cr = op[tkm2].re;
        ci = op[tkm2].im;

        dr = op[tkm3].re;
        di = op[tkm3].im;

        er = op[tkm4].re;
        ei = op[tkm4].im;

        fr = op[tkm5].re;
        fi = op[tkm5].im;

        gr = op[tkm6].re;
        gi = op[tkm6].im;

        tau0r = br + gr;
        tau3r = br - gr;
        tau0i = bi + gi;
        tau3i = bi - gi;

        tau1r = cr + fr;
        tau4r = cr - fr;
        tau1i = ci + fi;
        tau4i = ci - fi;

        tau2r = dr + er;
        tau5r = dr - er;
        tau2i = di + ei;
        tau5i = di - ei;

        op[0].re = ar + tau0r + tau1r + tau2r;
        op[0].im = ai + tau0i + tau1i + tau2i;

        tau6r = ar + c1 * tau0r + c2 * tau1r + c3 * tau2r;
        tau6i = ai + c1 * tau0i + c2 * tau1i + c3 * tau2i;


        if (sgn == 1) {
            tau7r = -s1 * tau3r - s2 * tau4r - s3 * tau5r;
            tau7i = -s1 * tau3i - s2 * tau4i - s3 * tau5i;

        } else {
            tau7r = s1 * tau3r + s2 * tau4r + s3 * tau5r;
            tau7i = s1 * tau3i + s2 * tau4i + s3 * tau5i;

        }


        op[tkm1].re = tau6r - tau7i;
        op[tkm1].im = tau6i + tau7r;

        op[tkm6].re = tau6r + tau7i;
        op[tkm6].im = tau6i - tau7r;

        tau6r = ar + c2 * tau0r + c3 * tau1r + c1 * tau2r;
        tau6i = ai + c2 * tau0i + c3 * tau1i + c1 * tau2i;


        if (sgn == 1) {
            tau7r = -s2 * tau3r + s3 * tau4r + s1 * tau5r;
            tau7i = -s2 * tau3i + s3 * tau4i + s1 * tau5i;

        } else {
            tau7r = s2 * tau3r - s3 * tau4r - s1 * tau5r;
            tau7i = s2 * tau3i - s3 * tau4i - s1 * tau5i;

        }


        op[tkm2].re = tau6r - tau7i;
        op[tkm2].im = tau6i + tau7r;

        op[tkm5].re = tau6r + tau7i;
        op[tkm5].im = tau6i - tau7r;

        tau6r = ar + c3 * tau0r + c1 * tau1r + c2 * tau2r;
        tau6i = ai + c3 * tau0i + c1 * tau1i + c2 * tau2i;

        if (sgn == 1) {
            tau7r = -s3 * tau3r + s1 * tau4r - s2 * tau5r;
            tau7i = -s3 * tau3i + s1 * tau4i - s2 * tau5i;

        } else {
            tau7r = s3 * tau3r - s1 * tau4r + s2 * tau5r;
            tau7i = s3 * tau3i - s1 * tau4i + s2 * tau5i;

        }


        op[tkm3].re = tau6r - tau7i;
        op[tkm3].im = tau6i + tau7r;

        op[tkm4].re = tau6r + tau7i;
        op[tkm4].im = tau6i - tau7r;


        for (k = 1; k < m; k++) {
            ind = m - 1 + 6 * k;
            wlr = (obj->twiddle + ind)->re;
            wli = (obj->twiddle + ind)->im;
            ind++;
            wl2r = (obj->twiddle + ind)->re;
            wl2i = (obj->twiddle + ind)->im;
            ind++;
            wl3r = (obj->twiddle + ind)->re;
            wl3i = (obj->twiddle + ind)->im;
            ind++;
            wl4r = (obj->twiddle + ind)->re;
            wl4i = (obj->twiddle + ind)->im;
            ind++;
            wl5r = (obj->twiddle + ind)->re;
            wl5i = (obj->twiddle + ind)->im;
            ind++;
            wl6r = (obj->twiddle + ind)->re;
            wl6i = (obj->twiddle + ind)->im;

            tkm1 = k + m;
            tkm2 = tkm1 + m;
            tkm3 = tkm2 + m;
            tkm4 = tkm3 + m;
            tkm5 = tkm4 + m;
            tkm6 = tkm5 + m;

            ar = op[k].re;
            ai = op[k].im;

            br = op[tkm1].re * wlr - op[tkm1].im * wli;
            bi = op[tkm1].im * wlr + op[tkm1].re * wli;

            cr = op[tkm2].re * wl2r - op[tkm2].im * wl2i;
            ci = op[tkm2].im * wl2r + op[tkm2].re * wl2i;

            dr = op[tkm3].re * wl3r - op[tkm3].im * wl3i;
            di = op[tkm3].im * wl3r + op[tkm3].re * wl3i;

            er = op[tkm4].re * wl4r - op[tkm4].im * wl4i;
            ei = op[tkm4].im * wl4r + op[tkm4].re * wl4i;

            fr = op[tkm5].re * wl5r - op[tkm5].im * wl5i;
            fi = op[tkm5].im * wl5r + op[tkm5].re * wl5i;

            gr = op[tkm6].re * wl6r - op[tkm6].im * wl6i;
            gi = op[tkm6].im * wl6r + op[tkm6].re * wl6i;

            tau0r = br + gr;
            tau3r = br - gr;
            tau0i = bi + gi;
            tau3i = bi - gi;

            tau1r = cr + fr;
            tau4r = cr - fr;
            tau1i = ci + fi;
            tau4i = ci - fi;

            tau2r = dr + er;
            tau5r = dr - er;
            tau2i = di + ei;
            tau5i = di - ei;

            op[k].re = ar + tau0r + tau1r + tau2r;
            op[k].im = ai + tau0i + tau1i + tau2i;

            tau6r = ar + c1 * tau0r + c2 * tau1r + c3 * tau2r;
            tau6i = ai + c1 * tau0i + c2 * tau1i + c3 * tau2i;

            if (sgn == 1) {
                tau7r = -s1 * tau3r - s2 * tau4r - s3 * tau5r;
                tau7i = -s1 * tau3i - s2 * tau4i - s3 * tau5i;

            } else {
                tau7r = s1 * tau3r + s2 * tau4r + s3 * tau5r;
                tau7i = s1 * tau3i + s2 * tau4i + s3 * tau5i;

            }


            op[tkm1].re = tau6r - tau7i;
            op[tkm1].im = tau6i + tau7r;

            op[tkm6].re = tau6r + tau7i;
            op[tkm6].im = tau6i - tau7r;

            tau6r = ar + c2 * tau0r + c3 * tau1r + c1 * tau2r;
            tau6i = ai + c2 * tau0i + c3 * tau1i + c1 * tau2i;


            if (sgn == 1) {
                tau7r = -s2 * tau3r + s3 * tau4r + s1 * tau5r;
                tau7i = -s2 * tau3i + s3 * tau4i + s1 * tau5i;

            } else {
                tau7r = s2 * tau3r - s3 * tau4r - s1 * tau5r;
                tau7i = s2 * tau3i - s3 * tau4i - s1 * tau5i;

            }


            op[tkm2].re = tau6r - tau7i;
            op[tkm2].im = tau6i + tau7r;

            op[tkm5].re = tau6r + tau7i;
            op[tkm5].im = tau6i - tau7r;

            tau6r = ar + c3 * tau0r + c1 * tau1r + c2 * tau2r;
            tau6i = ai + c3 * tau0i + c1 * tau1i + c2 * tau2i;


            if (sgn == 1) {
                tau7r = -s3 * tau3r + s1 * tau4r - s2 * tau5r;
                tau7i = -s3 * tau3i + s1 * tau4i - s2 * tau5i;

            } else {
                tau7r = s3 * tau3r - s1 * tau4r + s2 * tau5r;
                tau7i = s3 * tau3i - s1 * tau4i + s2 * tau5i;

            }


            op[tkm3].re = tau6r - tau7i;
            op[tkm3].im = tau6i + tau7r;

            op[tkm4].re = tau6r + tau7i;
            op[tkm4].im = tau6i - tau7r;

        }

    } else if (radix == 8) {
        int k, tkm1, tkm2, tkm3, tkm4, tkm5, tkm6, tkm7, ind;
        float wlr, wli, wl2r, wl2i, wl3r, wl3i, wl4r, wl4i, wl5r, wl5i, wl6r, wl6i, wl7r, wl7i;
        float tau0r, tau0i, tau1r, tau1i, tau2r, tau2i, tau3r, tau3i;
        float ar, ai, br, bi, cr, ci, dr, di, er, ei, fr, fi, gr, gi, hr, hi;
        float tau4r, tau4i, tau5r, tau5i, tau6r, tau6i, tau7r, tau7i, tau8r, tau8i, tau9r, tau9i;
        float c1, s1, temp1r, temp1i, temp2r, temp2i;
        m = N / 8;
        ll = 8 * l;
        mixed_radix_dit_rec(op, ip, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + m, ip + l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 2 * m, ip + 2 * l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 3 * m, ip + 3 * l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 4 * m, ip + 4 * l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 5 * m, ip + 5 * l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 6 * m, ip + 6 * l, obj, sgn, m, ll, inc + 1);
        mixed_radix_dit_rec(op + 7 * m, ip + 7 * l, obj, sgn, m, ll, inc + 1);

        c1 = 0.70710678118654752440084436210485f;
        s1 = 0.70710678118654752440084436210485f;


        for (k = 0; k < m; k++) {
            ind = m - 1 + 7 * k;
            wlr = (obj->twiddle + ind)->re;
            wli = (obj->twiddle + ind)->im;
            ind++;
            wl2r = (obj->twiddle + ind)->re;
            wl2i = (obj->twiddle + ind)->im;
            ind++;
            wl3r = (obj->twiddle + ind)->re;
            wl3i = (obj->twiddle + ind)->im;
            ind++;
            wl4r = (obj->twiddle + ind)->re;
            wl4i = (obj->twiddle + ind)->im;
            ind++;
            wl5r = (obj->twiddle + ind)->re;
            wl5i = (obj->twiddle + ind)->im;
            ind++;
            wl6r = (obj->twiddle + ind)->re;
            wl6i = (obj->twiddle + ind)->im;
            ind++;
            wl7r = (obj->twiddle + ind)->re;
            wl7i = (obj->twiddle + ind)->im;

            tkm1 = k + m;
            tkm2 = tkm1 + m;
            tkm3 = tkm2 + m;
            tkm4 = tkm3 + m;
            tkm5 = tkm4 + m;
            tkm6 = tkm5 + m;
            tkm7 = tkm6 + m;

            ar = op[k].re;
            ai = op[k].im;

            br = op[tkm1].re * wlr - op[tkm1].im * wli;
            bi = op[tkm1].im * wlr + op[tkm1].re * wli;

            cr = op[tkm2].re * wl2r - op[tkm2].im * wl2i;
            ci = op[tkm2].im * wl2r + op[tkm2].re * wl2i;

            dr = op[tkm3].re * wl3r - op[tkm3].im * wl3i;
            di = op[tkm3].im * wl3r + op[tkm3].re * wl3i;

            er = op[tkm4].re * wl4r - op[tkm4].im * wl4i;
            ei = op[tkm4].im * wl4r + op[tkm4].re * wl4i;

            fr = op[tkm5].re * wl5r - op[tkm5].im * wl5i;
            fi = op[tkm5].im * wl5r + op[tkm5].re * wl5i;

            gr = op[tkm6].re * wl6r - op[tkm6].im * wl6i;
            gi = op[tkm6].im * wl6r + op[tkm6].re * wl6i;

            hr = op[tkm7].re * wl7r - op[tkm7].im * wl7i;
            hi = op[tkm7].im * wl7r + op[tkm7].re * wl7i;

            tau0r = ar + er;
            tau4r = ar - er;
            tau0i = ai + ei;
            tau4i = ai - ei;

            tau1r = br + hr;
            tau5r = br - hr;
            tau1i = bi + hi;
            tau5i = bi - hi;

            tau2r = dr + fr;
            tau6r = dr - fr;
            tau6i = di - fi;
            tau2i = di + fi;

            tau3r = cr + gr;
            tau7r = cr - gr;
            tau7i = ci - gi;
            tau3i = ci + gi;

            op[k].re = tau0r + tau1r + tau2r + tau3r;
            op[k].im = tau0i + tau1i + tau2i + tau3i;

            op[tkm4].re = tau0r - tau1r - tau2r + tau3r;
            op[tkm4].im = tau0i - tau1i - tau2i + tau3i;

            temp1r = tau1r - tau2r;
            temp1i = tau1i - tau2i;

            temp2r = tau5r + tau6r;
            temp2i = tau5i + tau6i;

            tau8r = tau4r + c1 * temp1r;
            tau8i = tau4i + c1 * temp1i;

            if (sgn == 1) {
                tau9r = -s1 * temp2r - tau7r;
                tau9i = -s1 * temp2i - tau7i;

            } else {
                tau9r = s1 * temp2r + tau7r;
                tau9i = s1 * temp2i + tau7i;

            }


            op[tkm1].re = tau8r - tau9i;
            op[tkm1].im = tau8i + tau9r;

            op[tkm7].re = tau8r + tau9i;
            op[tkm7].im = tau8i - tau9r;

            tau8r = tau0r - tau3r;
            tau8i = tau0i - tau3i;

            if (sgn == 1) {
                tau9r = -tau5r + tau6r;
                tau9i = -tau5i + tau6i;

            } else {
                tau9r = tau5r - tau6r;
                tau9i = tau5i - tau6i;

            }


            op[tkm2].re = tau8r - tau9i;
            op[tkm2].im = tau8i + tau9r;

            op[tkm6].re = tau8r + tau9i;
            op[tkm6].im = tau8i - tau9r;

            tau8r = tau4r - c1 * temp1r;
            tau8i = tau4i - c1 * temp1i;

            //tau9r = sgn * ( -s1 * temp2r + tau7r);
            //tau9i = sgn * ( -s1 * temp2i + tau7i);

            if (sgn == 1) {
                tau9r = -s1 * temp2r + tau7r;
                tau9i = -s1 * temp2i + tau7i;

            } else {
                tau9r = s1 * temp2r - tau7r;
                tau9i = s1 * temp2i - tau7i;

            }


            op[tkm3].re = tau8r - tau9i;
            op[tkm3].im = tau8i + tau9r;

            op[tkm5].re = tau8r + tau9i;
            op[tkm5].im = tau8i - tau9r;

        }

    } else {
        int k, i, ind;
        int M, tkm, u, v, t, tt;
        float temp1r, temp1i, temp2r, temp2i;
        float *wlr = (float *) malloc(sizeof(float) * (radix - 1));
        float *wli = (float *) malloc(sizeof(float) * (radix - 1));
        float *taur = (float *) malloc(sizeof(float) * (radix - 1));
        float *taui = (float *) malloc(sizeof(float) * (radix - 1));
        float *c1 = (float *) malloc(sizeof(float) * (radix - 1));
        float *s1 = (float *) malloc(sizeof(float) * (radix - 1));
        float *yr = (float *) malloc(sizeof(float) * (radix));
        float *yi = (float *) malloc(sizeof(float) * (radix));

        m = N / radix;
        ll = radix * l;

        for (i = 0; i < radix; ++i) {
            mixed_radix_dit_rec(op + i * m, ip + i * l, obj, sgn, m, ll, inc + 1);
        }

        M = (radix - 1) / 2;

        for (i = 1; i < M + 1; ++i) {
            c1[i - 1] = cosf(i * PI2 / radix);
            s1[i - 1] = sinf(i * PI2 / radix);
        }

        for (i = 0; i < M; ++i) {
            s1[i + M] = -s1[M - 1 - i];
            c1[i + M] = c1[M - 1 - i];
        }

        for (k = 0; k < m; ++k) {
            ind = m - 1 + (radix - 1) * k;
            yr[0] = op[k].re;
            yi[0] = op[k].im;
            for (i = 0; i < radix - 1; ++i) {
                wlr[i] = (obj->twiddle + ind)->re;
                wli[i] = (obj->twiddle + ind)->im;
                tkm = k + (i + 1) * m;
                yr[i + 1] = op[tkm].re * wlr[i] - op[tkm].im * wli[i];
                yi[i + 1] = op[tkm].im * wlr[i] + op[tkm].re * wli[i];
                ind++;
            }

            for (i = 0; i < M; ++i) {
                taur[i] = yr[i + 1] + yr[radix - 1 - i];
                taui[i + M] = yi[i + 1] - yi[radix - 1 - i];
                taui[i] = yi[i + 1] + yi[radix - 1 - i];
                taur[i + M] = yr[i + 1] - yr[radix - 1 - i];
            }

            temp1r = yr[0];
            temp1i = yi[0];

            for (i = 0; i < M; ++i) {
                temp1r += taur[i];
                temp1i += taui[i];
            }

            op[k].re = temp1r;
            op[k].im = temp1i;

            for (u = 0; u < M; u++) {
                temp1r = yr[0];
                temp1i = yi[0];
                temp2r = 0.0;
                temp2i = 0.0;
                for (v = 0; v < M; v++) {
                    t = (u + 1) * (v + 1);
                    while (t >= radix)
                        t -= radix;
                    tt = t - 1;

                    temp1r += c1[tt] * taur[v];
                    temp1i += c1[tt] * taui[v];
                    temp2r -= s1[tt] * taur[v + M];
                    temp2i -= s1[tt] * taui[v + M];
                }
                temp2r = sgn * temp2r;
                temp2i = sgn * temp2i;


                op[k + (u + 1) * m].re = temp1r - temp2i;
                op[k + (u + 1) * m].im = temp1i + temp2r;

                op[k + (radix - u - 1) * m].re = temp1r + temp2i;
                op[k + (radix - u - 1) * m].im = temp1i - temp2r;
            }


        }
        free(wlr);
        free(wli);
        free(taur);
        free(taui);
        free(c1);
        free(s1);
        free(yr);
        free(yi);

    }


}

static void bluestein_exp(fft_t *hl, fft_t *hlt, int len, int M) {
    float PI, theta, angle;
    int l2, len2, i;
    PI = 3.1415926535897932384626433832795f;
    theta = PI / len;
    l2 = 0;
    len2 = 2 * len;

    for (i = 0; i < len; ++i) {
        angle = theta * l2;
        hlt[i].re = cosf(angle);
        hlt[i].im = sinf(angle);
        hl[i].re = hlt[i].re;
        hl[i].im = hlt[i].im;
        l2 += 2 * i + 1;
        while (l2 > len2) {
            l2 -= len2;
        }

    }

    for (i = len; i < M - len + 1; i++) {
        hl[i].re = 0.0;
        hl[i].im = 0.0;
    }

    for (i = M - len + 1; i < M; i++) {
        hl[i].re = hlt[M - i].re;
        hl[i].im = hlt[M - i].im;
    }

}

static void bluestein_fft(fft_t *data, fft_t *oup, fft_object obj, int sgn, int N) {

    int K, M, ii, i;
    int def_lt, def_N, def_sgn;
    float scale, temp;
    fft_t *yn;
    fft_t *hk;
    fft_t *tempop;
    fft_t *yno;
    fft_t *hlt;
    obj->lt = 0;
    K = (int) powf(2.0f, ceilf(log10f((float) N) / log10f((float) 2.0f)));
    def_lt = 1;
    def_sgn = obj->sgn;
    def_N = obj->N;

    if (K < 2 * N - 2) {
        M = K * 2;
    } else {
        M = K;
    }
    obj->N = M;

    yn = (fft_t *) malloc(sizeof(fft_t) * M);
    hk = (fft_t *) malloc(sizeof(fft_t) * M);
    tempop = (fft_t *) malloc(sizeof(fft_t) * M);
    yno = (fft_t *) malloc(sizeof(fft_t) * M);
    hlt = (fft_t *) malloc(sizeof(fft_t) * N);
    bluestein_exp(tempop, hlt, N, M);
    scale = 1.0f / M;

    for (ii = 0; ii < M; ++ii) {
        tempop[ii].im *= scale;
        tempop[ii].re *= scale;
    }

    fft_exec(obj, tempop, hk);

    if (sgn == 1) {
        for (i = 0; i < N; i++) {
            tempop[i].re = data[i].re * hlt[i].re + data[i].im * hlt[i].im;
            tempop[i].im = -data[i].re * hlt[i].im + data[i].im * hlt[i].re;
        }
    } else {
        for (i = 0; i < N; i++) {
            tempop[i].re = data[i].re * hlt[i].re - data[i].im * hlt[i].im;
            tempop[i].im = data[i].re * hlt[i].im + data[i].im * hlt[i].re;
        }
    }

    for (i = N; i < M; i++) {
        tempop[i].re = 0.0;
        tempop[i].im = 0.0;
    }

    fft_exec(obj, tempop, yn);

    if (sgn == 1) {
        for (i = 0; i < M; i++) {
            temp = yn[i].re * hk[i].re - yn[i].im * hk[i].im;
            yn[i].im = yn[i].re * hk[i].im + yn[i].im * hk[i].re;
            yn[i].re = temp;
        }
    } else {
        for (i = 0; i < M; i++) {
            temp = yn[i].re * hk[i].re + yn[i].im * hk[i].im;
            yn[i].im = -yn[i].re * hk[i].im + yn[i].im * hk[i].re;
            yn[i].re = temp;
        }

    }

    //IFFT

    for (ii = 0; ii < M; ++ii) {
        (obj->twiddle + ii)->im = -(obj->twiddle + ii)->im;
    }

    obj->sgn = -1 * sgn;

    fft_exec(obj, yn, yno);

    if (sgn == 1) {
        for (i = 0; i < N; i++) {
            oup[i].re = yno[i].re * hlt[i].re + yno[i].im * hlt[i].im;
            oup[i].im = -yno[i].re * hlt[i].im + yno[i].im * hlt[i].re;
        }
    } else {
        for (i = 0; i < N; i++) {
            oup[i].re = yno[i].re * hlt[i].re - yno[i].im * hlt[i].im;
            oup[i].im = yno[i].re * hlt[i].im + yno[i].im * hlt[i].re;
        }

    }

    obj->sgn = def_sgn;
    obj->N = def_N;
    obj->lt = def_lt;
    for (ii = 0; ii < M; ++ii) {
        (obj->twiddle + ii)->im = -(obj->twiddle + ii)->im;
    }

    free(yn);
    free(yno);
    free(tempop);
    free(hk);
    free(hlt);

}


void fft_exec(fft_object obj, fft_t *inp, fft_t *oup) {
    if (obj->lt == 0) {
        int l, inc;
        int nn, sgn1;
        nn = obj->N;
        sgn1 = obj->sgn;
        l = 1;
        inc = 0;
        mixed_radix_dit_rec(oup, inp, obj, sgn1, nn, l, inc);
    } else if (obj->lt == 1) {
        int nn, sgn1;
        nn = obj->N;
        sgn1 = obj->sgn;
        bluestein_fft(inp, oup, obj, sgn1, nn);

    }

}

int divideby(int M, int d) {
    while (M % d == 0) {
        M = M / d;
    }
    if (M == 1) {
        return 1;
    }
    return 0;
}

int dividebyN(int N) {
    while (N % 53 == 0) {
        N = N / 53;
    }
    while (N % 47 == 0) {
        N = N / 47;
    }
    while (N % 43 == 0) {
        N = N / 43;
    }
    while (N % 41 == 0) {
        N = N / 41;
    }
    while (N % 37 == 0) {
        N = N / 37;
    }
    while (N % 31 == 0) {
        N = N / 31;
    }
    while (N % 29 == 0) {
        N = N / 29;
    }
    while (N % 23 == 0) {
        N = N / 23;
    }
    while (N % 17 == 0) {
        N = N / 17;
    }
    while (N % 13 == 0) {
        N = N / 13;
    }
    while (N % 11 == 0) {
        N = N / 11;
    }
    while (N % 8 == 0) {
        N = N / 8;
    }
    while (N % 7 == 0) {
        N = N / 7;
    }
    while (N % 5 == 0) {
        N = N / 5;
    }
    while (N % 4 == 0) {
        N = N / 4;
    }
    while (N % 3 == 0) {
        N = N / 3;
    }
    while (N % 2 == 0) {
        N = N / 2;
    }
    if (N == 1) {
        return 1;
    }
    return 0;

}

int factors(int M, int *arr) {
    int i, N, num, mult, m1, m2;
    i = 0;
    N = M;
    while (N % 53 == 0) {
        N = N / 53;
        arr[i] = 53;
        i++;
    }
    while (N % 47 == 0) {
        N = N / 47;
        arr[i] = 47;
        i++;
    }
    while (N % 43 == 0) {
        N = N / 43;
        arr[i] = 43;
        i++;
    }
    while (N % 41 == 0) {
        N = N / 41;
        arr[i] = 41;
        i++;
    }
    while (N % 37 == 0) {
        N = N / 37;
        arr[i] = 37;
        i++;
    }
    while (N % 31 == 0) {
        N = N / 31;
        arr[i] = 31;
        i++;
    }
    while (N % 29 == 0) {
        N = N / 29;
        arr[i] = 29;
        i++;
    }
    while (N % 23 == 0) {
        N = N / 23;
        arr[i] = 23;
        i++;
    }
    while (N % 19 == 0) {
        N = N / 19;
        arr[i] = 19;
        i++;
    }
    while (N % 17 == 0) {
        N = N / 17;
        arr[i] = 17;
        i++;
    }
    while (N % 13 == 0) {
        N = N / 13;
        arr[i] = 13;
        i++;
    }
    while (N % 11 == 0) {
        N = N / 11;
        arr[i] = 11;
        i++;
    }
    while (N % 8 == 0) {
        N = N / 8;
        arr[i] = 8;
        i++;
    }
    while (N % 7 == 0) {
        N = N / 7;
        arr[i] = 7;
        i++;
    }
    while (N % 5 == 0) {
        N = N / 5;
        arr[i] = 5;
        i++;
    }
    while (N % 4 == 0) {
        N = N / 4;
        arr[i] = 4;
        i++;
    }
    while (N % 3 == 0) {
        N = N / 3;
        arr[i] = 3;
        i++;
    }
    while (N % 2 == 0) {
        N = N / 2;
        arr[i] = 2;
        i++;
    }
    if (N > 31) {
        num = 2;

        while (N > 1) {
            mult = num * 6;
            m1 = mult - 1;
            m2 = mult + 1;
            while (N % m1 == 0) {
                arr[i] = m1;
                i++;
                N = N / m1;
            }
            while (N % m2 == 0) {
                arr[i] = m2;
                i++;
                N = N / m2;
            }
            num += 1;

        }
    }
    return i;

}


void twiddle(fft_t *vec, int N, int radix) {
    int K, KL;
    float theta, theta2;
    theta = PI2 / N;
    KL = N / radix;
    vec[0].re = 1.0f;
    vec[0].im = 0.0f;

    for (K = 1; K < KL; K++) {
        theta2 = theta * K;
        vec[K].re = cosf(theta2);
        vec[K].im = -sinf(theta2);
    }

}

void longvectorN(fft_t *sig, int N, int *array, int tx) {
    int L, i, Ls, ct, j, k;
    float theta;
    L = 1;
    ct = 0;
    for (i = 0; i < tx; i++) {
        L = L * array[tx - 1 - i];
        Ls = L / array[tx - 1 - i];
        theta = -1.0f * PI2 / L;
        for (j = 0; j < Ls; j++) {
            for (k = 0; k < array[tx - 1 - i] - 1; k++) {
                sig[ct].re = cosf((k + 1) * j * theta);
                sig[ct].im = sinf((k + 1) * j * theta);
                ct++;
            }
        }

    }

}


void free_fft(fft_object object) {
    free(object);
}