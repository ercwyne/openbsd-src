/* $OpenBSD: ec_asn1_test.c,v 1.6 2024/10/16 23:58:25 tb Exp $ */
/*
 * Copyright (c) 2017, 2021 Joel Sing <jsing@openbsd.org>
 * Copyright (c) 2024 Theo Buehler <tb@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <err.h>
#include <string.h>

#include <openssl/bio.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/objects.h>

const uint8_t ec_secp256r1_pkparameters_named_curve[] = {
	0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03,
	0x01, 0x07,
};

const uint8_t ec_secp256r1_pkparameters_parameters[] = {
	0x30, 0x81, 0xf7, 0x02, 0x01, 0x01, 0x30, 0x2c,
	0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x01,
	0x01, 0x02, 0x21, 0x00, 0xff, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0x30, 0x5b, 0x04, 0x20,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,
	0x04, 0x20, 0x5a, 0xc6, 0x35, 0xd8, 0xaa, 0x3a,
	0x93, 0xe7, 0xb3, 0xeb, 0xbd, 0x55, 0x76, 0x98,
	0x86, 0xbc, 0x65, 0x1d, 0x06, 0xb0, 0xcc, 0x53,
	0xb0, 0xf6, 0x3b, 0xce, 0x3c, 0x3e, 0x27, 0xd2,
	0x60, 0x4b, 0x03, 0x15, 0x00, 0xc4, 0x9d, 0x36,
	0x08, 0x86, 0xe7, 0x04, 0x93, 0x6a, 0x66, 0x78,
	0xe1, 0x13, 0x9d, 0x26, 0xb7, 0x81, 0x9f, 0x7e,
	0x90, 0x04, 0x41, 0x04, 0x6b, 0x17, 0xd1, 0xf2,
	0xe1, 0x2c, 0x42, 0x47, 0xf8, 0xbc, 0xe6, 0xe5,
	0x63, 0xa4, 0x40, 0xf2, 0x77, 0x03, 0x7d, 0x81,
	0x2d, 0xeb, 0x33, 0xa0, 0xf4, 0xa1, 0x39, 0x45,
	0xd8, 0x98, 0xc2, 0x96, 0x4f, 0xe3, 0x42, 0xe2,
	0xfe, 0x1a, 0x7f, 0x9b, 0x8e, 0xe7, 0xeb, 0x4a,
	0x7c, 0x0f, 0x9e, 0x16, 0x2b, 0xce, 0x33, 0x57,
	0x6b, 0x31, 0x5e, 0xce, 0xcb, 0xb6, 0x40, 0x68,
	0x37, 0xbf, 0x51, 0xf5, 0x02, 0x21, 0x00, 0xff,
	0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbc,
	0xe6, 0xfa, 0xad, 0xa7, 0x17, 0x9e, 0x84, 0xf3,
	0xb9, 0xca, 0xc2, 0xfc, 0x63, 0x25, 0x51, 0x02,
	0x01, 0x01,
};

const uint8_t ec_secp256k1_pkparameters_parameters[] = {
	0x30, 0x81, 0xe0, 0x02, 0x01, 0x01, 0x30, 0x2c,
	0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x01,
	0x01, 0x02, 0x21, 0x00, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
	0xff, 0xff, 0xfc, 0x2f, 0x30, 0x44, 0x04, 0x20,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x04, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x07, 0x04, 0x41, 0x04, 0x79, 0xbe, 0x66,
	0x7e, 0xf9, 0xdc, 0xbb, 0xac, 0x55, 0xa0, 0x62,
	0x95, 0xce, 0x87, 0x0b, 0x07, 0x02, 0x9b, 0xfc,
	0xdb, 0x2d, 0xce, 0x28, 0xd9, 0x59, 0xf2, 0x81,
	0x5b, 0x16, 0xf8, 0x17, 0x98, 0x48, 0x3a, 0xda,
	0x77, 0x26, 0xa3, 0xc4, 0x65, 0x5d, 0xa4, 0xfb,
	0xfc, 0x0e, 0x11, 0x08, 0xa8, 0xfd, 0x17, 0xb4,
	0x48, 0xa6, 0x85, 0x54, 0x19, 0x9c, 0x47, 0xd0,
	0x8f, 0xfb, 0x10, 0xd4, 0xb8, 0x02, 0x21, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
	0xba, 0xae, 0xdc, 0xe6, 0xaf, 0x48, 0xa0, 0x3b,
	0xbf, 0xd2, 0x5e, 0x8c, 0xd0, 0x36, 0x41, 0x41,
	0x02, 0x01, 0x01,
};

static void
hexdump(const unsigned char *buf, size_t len)
{
	size_t i;

	for (i = 1; i <= len; i++)
		fprintf(stderr, " 0x%02hhx,%s", buf[i - 1], i % 8 ? "" : "\n");

	fprintf(stderr, "\n");
}

static int
compare_data(const char *label, const unsigned char *d1, size_t d1_len,
    const unsigned char *d2, size_t d2_len)
{
	if (d1_len != d2_len) {
		fprintf(stderr, "FAIL: got %s with length %zu, want %zu\n",
		    label, d1_len, d2_len);
		return -1;
	}
	if (memcmp(d1, d2, d1_len) != 0) {
		fprintf(stderr, "FAIL: %sdiffer\n", label);
		fprintf(stderr, "got:\n");
		hexdump(d1, d1_len);
		fprintf(stderr, "want:\n");
		hexdump(d2, d2_len);
		return -1;
	}
	return 0;
}

static int
ec_group_pkparameters_test(const char *label, int nid, int asn1_flag,
    const uint8_t *test_data, size_t test_data_len)
{
	EC_GROUP *group_a = NULL, *group_b = NULL;
	unsigned char *out = NULL, *data = NULL;
	const unsigned char *p;
	BIO *bio_mem = NULL;
	int failure = 1;
	int len;

	/*
	 * Test i2d_ECPKParameters/d2i_ECPKParameters.
	 */
	if ((group_a = EC_GROUP_new_by_curve_name(nid)) == NULL)
		errx(1, "failed to create EC_GROUP");

	EC_GROUP_set_asn1_flag(group_a, asn1_flag);

	if ((len = i2d_ECPKParameters(group_a, &out)) < 0) {
		fprintf(stderr, "FAIL: i2d_ECPKParameters failed\n");
		goto done;
	}
	if (compare_data(label, out, len, test_data, test_data_len) == -1)
		goto done;

	p = out;
	if ((group_b = d2i_ECPKParameters(NULL, &p, len)) == NULL) {
		fprintf(stderr, "FAIL: d2i_ECPKParameters failed\n");
		goto done;
	}

	if (EC_GROUP_cmp(group_a, group_b, NULL) != 0) {
		fprintf(stderr, "FAIL: EC_GROUPs do not match!\n");
		goto done;
	}

	p = out;
	if ((group_a = d2i_ECPKParameters(&group_a, &p, len)) == NULL) {
		fprintf(stderr, "FAIL: d2i_ECPKParameters failed\n");
		goto done;
	}

	if (EC_GROUP_cmp(group_a, group_b, NULL) != 0) {
		fprintf(stderr, "FAIL: EC_GROUPs do not match!\n");
		goto done;
	}

	/*
	 * Test i2d_ECPKParameters_bio/d2i_ECPKParameters_bio.
	 */
	if ((bio_mem = BIO_new(BIO_s_mem())) == NULL)
                errx(1, "BIO_new failed for BIO_s_mem");

	if ((len = i2d_ECPKParameters_bio(bio_mem, group_a)) < 0) {
		fprintf(stderr, "FAIL: i2d_ECPKParameters_bio failed\n");
		goto done;
	}

	len = BIO_get_mem_data(bio_mem, &data);
	if (compare_data(label, out, len, test_data, test_data_len) == -1)
		goto done;

	EC_GROUP_free(group_b);
	if ((group_b = d2i_ECPKParameters_bio(bio_mem, NULL)) == NULL) {
		fprintf(stderr, "FAIL: d2i_ECPKParameters_bio failed\n");
		goto done;
	}

	if (EC_GROUP_cmp(group_a, group_b, NULL) != 0) {
		fprintf(stderr, "FAIL: EC_GROUPs do not match!\n");
		goto done;
	}

	failure = 0;

 done:
	BIO_free_all(bio_mem);
	EC_GROUP_free(group_a);
	EC_GROUP_free(group_b);
	free(out);

	return (failure);
}

static int
ec_group_pkparameters_named_curve_test(void)
{
	return ec_group_pkparameters_test("ECPKPARAMETERS named curve",
	    NID_X9_62_prime256v1, OPENSSL_EC_NAMED_CURVE,
	    ec_secp256r1_pkparameters_named_curve,
	    sizeof(ec_secp256r1_pkparameters_named_curve));
}

static int
ec_group_pkparameters_parameters_test(void)
{
	return ec_group_pkparameters_test("ECPKPARAMETERS parameters",
	    NID_X9_62_prime256v1, OPENSSL_EC_EXPLICIT_CURVE,
	    ec_secp256r1_pkparameters_parameters,
	    sizeof(ec_secp256r1_pkparameters_parameters));
}

static int
ec_group_pkparameters_correct_padding_test(void)
{
	return ec_group_pkparameters_test("ECPKPARAMETERS parameters",
	    NID_secp256k1, OPENSSL_EC_EXPLICIT_CURVE,
	    ec_secp256k1_pkparameters_parameters,
	    sizeof(ec_secp256k1_pkparameters_parameters));
}

static int
ec_group_roundtrip_curve(const EC_GROUP *group, const char *descr, int nid)
{
	EC_GROUP *new_group = NULL;
	unsigned char *der = NULL;
	int der_len;
	const unsigned char *p;
	int failed = 1;

	der = NULL;
	if ((der_len = i2d_ECPKParameters(group, &der)) <= 0)
		errx(1, "failed to serialize %s %d", descr, nid);

	p = der;
	if ((new_group = d2i_ECPKParameters(NULL, &p, der_len)) == NULL)
		errx(1, "failed to deserialize %s %d", descr, nid);

	if (EC_GROUP_cmp(group, new_group, NULL) != 0) {
		fprintf(stderr, "FAIL: %s %d groups mismatch\n", descr, nid);
		goto err;
	}
	if (EC_GROUP_get_asn1_flag(group) != EC_GROUP_get_asn1_flag(new_group)) {
		fprintf(stderr, "FAIL: %s %d asn1_flag %x != %x\n", descr, nid,
		    EC_GROUP_get_asn1_flag(group),
		    EC_GROUP_get_asn1_flag(new_group));
		goto err;
	}
	if (EC_GROUP_get_point_conversion_form(group) !=
	    EC_GROUP_get_point_conversion_form(new_group)) {
		fprintf(stderr, "FAIL: %s %d form %02x != %02x\n", descr, nid,
		    EC_GROUP_get_point_conversion_form(group),
		    EC_GROUP_get_point_conversion_form(new_group));
		goto err;
	}

	failed = 0;

 err:
	EC_GROUP_free(new_group);
	free(der);

	return failed;
}

static int
ec_group_roundtrip_builtin_curve(const EC_builtin_curve *curve)
{
	EC_GROUP *group = NULL;
	int failed = 0;

	if ((group = EC_GROUP_new_by_curve_name(curve->nid)) == NULL)
		errx(1, "failed to instantiate curve %d", curve->nid);

	if (EC_GROUP_get_asn1_flag(group) != OPENSSL_EC_NAMED_CURVE) {
		fprintf(stderr, "FAIL: ASN.1 flag not set for %d\n", curve->nid);
		goto err;
	}
	if (EC_GROUP_get_point_conversion_form(group) !=
	    POINT_CONVERSION_UNCOMPRESSED) {
		fprintf(stderr, "FAIL: %d has point conversion form %02x\n",
		    curve->nid, EC_GROUP_get_point_conversion_form(group));
		goto err;
	}

	failed |= ec_group_roundtrip_curve(group, "named", curve->nid);

	EC_GROUP_set_asn1_flag(group, 0);
	failed |= ec_group_roundtrip_curve(group, "explicit", curve->nid);

	EC_GROUP_set_point_conversion_form(group, POINT_CONVERSION_COMPRESSED);
	failed |= ec_group_roundtrip_curve(group, "compressed", curve->nid);

	EC_GROUP_set_point_conversion_form(group, POINT_CONVERSION_HYBRID);
	failed |= ec_group_roundtrip_curve(group, "hybrid", curve->nid);

 err:
	EC_GROUP_free(group);

	return failed;
}

static int
ec_group_roundtrip_builtin_curves(void)
{
	EC_builtin_curve *all_curves = NULL;
	size_t curve_id, ncurves;
	int failed = 0;

	ncurves = EC_get_builtin_curves(NULL, 0);
	if ((all_curves = calloc(ncurves, sizeof(*all_curves))) == NULL)
		err(1, "calloc builtin curves");
	EC_get_builtin_curves(all_curves, ncurves);

	for (curve_id = 0; curve_id < ncurves; curve_id++)
		failed |= ec_group_roundtrip_builtin_curve(&all_curves[curve_id]);

	free(all_curves);

	return failed;
}

/*
 * From draft-ietf-lwig-curve-representation-23, Appendix E.3
 */

static const struct {
	const char *oid;
	const char *sn;
	const char *ln;
	const char *p;
	const char *a;
	const char *b;
	const char *order;
	const char *cofactor;
	const char *x;
	const char *y;
} wei25519 = {
	.oid = "1.3.101.108",
	.sn = "Wei25519",
	.p =	 "7fffffff" "ffffffff" "ffffffff" "ffffffff"
		 "ffffffff" "ffffffff" "ffffffff" "ffffffed",
	.a =	 "2aaaaaaa" "aaaaaaaa" "aaaaaaaa" "aaaaaaaa"
		 "aaaaaaaa" "aaaaaaaa" "aaaaaa98" "4914a144",
	.b =	 "7b425ed0" "97b425ed" "097b425e" "d097b425"
		 "ed097b42" "5ed097b4" "260b5e9c" "7710c864",
	.x =	 "2aaaaaaa" "aaaaaaaa" "aaaaaaaa" "aaaaaaaa"
		 "aaaaaaaa" "aaaaaaaa" "aaaaaaaa" "aaad245a",
	.y =	 "20ae19a1" "b8a086b4" "e01edd2c" "7748d14c"
		 "923d4d7e" "6d7c61b2" "29e9c5a2" "7eced3d9",
	.order = "10000000" "00000000" "00000000" "00000000"
		 "14def9de" "a2f79cd6" "5812631a" "5cf5d3ed",
	.cofactor = "8",
};

const uint8_t ec_wei25519_pkparameters_named_curve[] = {
	0x06, 0x03, 0x2b, 0x65, 0x6c,
};

const uint8_t ec_wei25519_pkparameters_parameters[] = {
	0x30, 0x81, 0xde, 0x02, 0x01, 0x01, 0x30, 0x2b,
	0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x01,
	0x01, 0x02, 0x20, 0x7f, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xed, 0x30, 0x44, 0x04, 0x20, 0x2a,
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xaa, 0x98, 0x49, 0x14, 0xa1, 0x44, 0x04,
	0x20, 0x7b, 0x42, 0x5e, 0xd0, 0x97, 0xb4, 0x25,
	0xed, 0x09, 0x7b, 0x42, 0x5e, 0xd0, 0x97, 0xb4,
	0x25, 0xed, 0x09, 0x7b, 0x42, 0x5e, 0xd0, 0x97,
	0xb4, 0x26, 0x0b, 0x5e, 0x9c, 0x77, 0x10, 0xc8,
	0x64, 0x04, 0x41, 0x04, 0x2a, 0xaa, 0xaa, 0xaa,
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xad, 0x24, 0x5a, 0x20, 0xae, 0x19, 0xa1,
	0xb8, 0xa0, 0x86, 0xb4, 0xe0, 0x1e, 0xdd, 0x2c,
	0x77, 0x48, 0xd1, 0x4c, 0x92, 0x3d, 0x4d, 0x7e,
	0x6d, 0x7c, 0x61, 0xb2, 0x29, 0xe9, 0xc5, 0xa2,
	0x7e, 0xce, 0xd3, 0xd9, 0x02, 0x20, 0x10, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xde,
	0xf9, 0xde, 0xa2, 0xf7, 0x9c, 0xd6, 0x58, 0x12,
	0x63, 0x1a, 0x5c, 0xf5, 0xd3, 0xed, 0x02, 0x01,
	0x08,
};

static int
ec_weierstrass25519(void)
{
	EC_GROUP *group = NULL, *new_group = NULL;
	EC_POINT *generator = NULL;
	BN_CTX *ctx = NULL;
	BIGNUM *p, *a, *b;
	BIGNUM *order, *cofactor, *guessed_cofactor, *x, *y;
	const unsigned char *pder;
	unsigned char *der = NULL;
	long error;
	int der_len = 0;
	int nid;
	int failed = 1;

	ERR_clear_error();
	if ((ctx = BN_CTX_new()) == NULL)
		goto err;
	BN_CTX_start(ctx);

	if ((nid = OBJ_create(wei25519.oid, wei25519.sn, NULL)) == NID_undef) {
		fprintf(stderr, "FAIL: %s OBJ_create(wei25519)\n", __func__);
		goto err;
	}

	if ((p = BN_CTX_get(ctx)) == NULL)
		errx(1, "BN_CTX_get");
	if ((a = BN_CTX_get(ctx)) == NULL)
		errx(1, "BN_CTX_get");
	if ((b = BN_CTX_get(ctx)) == NULL)
		errx(1, "BN_CTX_get");
	if ((order = BN_CTX_get(ctx)) == NULL)
		errx(1, "BN_CTX_get");
	if ((cofactor = BN_CTX_get(ctx)) == NULL)
		errx(1, "BN_CTX_get");
	if ((guessed_cofactor = BN_CTX_get(ctx)) == NULL)
		errx(1, "BN_CTX_get");
	if ((x = BN_CTX_get(ctx)) == NULL)
		errx(1, "BN_CTX_get");
	if ((y = BN_CTX_get(ctx)) == NULL)
		errx(1, "BN_CTX_get");

	if (BN_hex2bn(&p, wei25519.p) == 0)
		errx(1, "BN_hex2bn(p)");
	if (BN_hex2bn(&a, wei25519.a) == 0)
		errx(1, "BN_hex2bn(a)");
	if (BN_hex2bn(&b, wei25519.b) == 0)
		errx(1, "BN_hex2bn(b)");

	/*
	 * XXX - this uses the Montgomery method. Consider exercising the
	 * simple method as well.
	 */
	if ((group = EC_GROUP_new_curve_GFp(p, a, b, ctx)) == NULL) {
		fprintf(stderr, "FAIL: %s EC_GROUP_new_curve_GFp", __func__);
		goto err;
	}

	if (BN_hex2bn(&x, wei25519.x) == 0)
		errx(1, "BN_hex2bn(x)");
	if (BN_hex2bn(&x, wei25519.x) == 0)
		errx(1, "BN_hex2bn(x)");
	if (BN_hex2bn(&y, wei25519.y) == 0)
		errx(1, "BN_hex2bn(y)");

	if ((generator = EC_POINT_new(group)) == NULL)
		errx(1, "EC_POINT_new()");

	if (!EC_POINT_set_affine_coordinates(group, generator, x, y, ctx)) {
		fprintf(stderr, "FAIL: %s EC_POINT_set_affine_coordinates", __func__);
		ERR_print_errors_fp(stderr);
		goto err;
	}

	if (BN_hex2bn(&order, wei25519.order) == 0)
		errx(1, "BN_hex2bn(order)");
	if (BN_hex2bn(&cofactor, wei25519.cofactor) == 0)
		errx(1, "BN_hex2bn(cofactor)");

	/* Don't set cofactor to exercise the cofactor guessing code. */
	if (!EC_GROUP_set_generator(group, generator, order, NULL)) {
		fprintf(stderr, "FAIL: %s EC_GROUP_set_generator\n", __func__);
		ERR_print_errors_fp(stderr);
		goto err;
	}

	if (!EC_GROUP_get_cofactor(group, guessed_cofactor, ctx)) {
		fprintf(stderr, "FAIL: %s EC_GROUP_get_cofactor\n", __func__);
		ERR_print_errors_fp(stderr);
		goto err;
	}

	if (BN_cmp(cofactor, guessed_cofactor) != 0) {
		fprintf(stderr, "FAIL: %s cofactor: want ", __func__);
		BN_print_fp(stderr, cofactor);
		fprintf(stderr, ", got ");
		BN_print_fp(stderr, guessed_cofactor);
		fprintf(stderr, "\n");
		goto err;
	}

	if (!EC_GROUP_check(group, ctx)) {
		fprintf(stderr, "FAIL: %s EC_GROUP_check\n", __func__);
		ERR_print_errors_fp(stderr);
		goto err;
	}

	/* Explicit curve parameter encoding should work without NID set. */
	if (EC_GROUP_get_curve_name(group) != NID_undef) {
		fprintf(stderr, "FAIL: %s unexpected curve name %d\n", __func__,
		    EC_GROUP_get_curve_name(group));
		ERR_print_errors_fp(stderr);
		goto err;
	}

	EC_GROUP_set_asn1_flag(group, OPENSSL_EC_EXPLICIT_CURVE);

	der = NULL;
	if ((der_len = i2d_ECPKParameters(group, &der)) <= 0) {
		fprintf(stderr, "FAIL: %s i2d_ECPKParameters (explicit)\n", __func__);
		ERR_print_errors_fp(stderr);
		goto err;
	}

	if (compare_data("Weierstrass 25519 explicit", der, der_len,
	    ec_wei25519_pkparameters_parameters,
	    sizeof(ec_wei25519_pkparameters_parameters)) == -1)
		goto err;

	freezero(der, der_len);
	der = NULL;

	EC_GROUP_set_curve_name(group, nid);
	EC_GROUP_set_asn1_flag(group, OPENSSL_EC_NAMED_CURVE);

	der = NULL;
	if ((der_len = i2d_ECPKParameters(group, &der)) <= 0) {
		fprintf(stderr, "FAIL: %s i2d_ECPKParameters (named)\n", __func__);
		ERR_print_errors_fp(stderr);
		goto err;
	}

	if (compare_data("Weierstrass 25519 named curve", der, der_len,
	    ec_wei25519_pkparameters_named_curve,
	    sizeof(ec_wei25519_pkparameters_named_curve)) == -1)
		goto err;

	freezero(der, der_len);
	der = NULL;

	/* At this point we should have no error on the stack. */
	if (ERR_peek_last_error() != 0) {
		fprintf(stderr, "FAIL: %s unexpected error %lu\n", __func__,
		    ERR_peek_last_error());
		goto err;
	}

	pder = ec_wei25519_pkparameters_named_curve;
	der_len = sizeof(ec_wei25519_pkparameters_named_curve);
	if ((new_group = d2i_ECPKParameters(NULL, &pder, der_len)) != NULL) {
		fprintf(stderr, "FAIL: %s managed to decode unknown named curve\n",
		    __func__);
		goto err;
	}

	error = ERR_get_error();
	if (ERR_GET_REASON(error) != EC_R_UNKNOWN_GROUP) {
		fprintf(stderr, "FAIL: %s unexpected error: want %d, got %d\n",
		    __func__, EC_R_UNKNOWN_GROUP, ERR_GET_REASON(error));
		goto err;
	}

	ERR_clear_error();
	pder = ec_wei25519_pkparameters_parameters;
	der_len = sizeof(ec_wei25519_pkparameters_parameters);

#if 0
	if ((new_group = d2i_ECPKParameters(NULL, &pder, der_len)) != NULL) {
		fprintf(stderr, "FAIL: %s managed to decode non-builtin parameters\n",
		    __func__);
		goto err;
	}

	error = ERR_peek_last_error();
	if (ERR_GET_REASON(error) != EC_R_PKPARAMETERS2GROUP_FAILURE) {
		fprintf(stderr, "FAIL: %s unexpected error: want %d, got %d\n",
		    __func__, EC_R_UNKNOWN_GROUP, ERR_GET_REASON(error));
		goto err;
	}
#else
	if ((new_group = d2i_ECPKParameters(NULL, &pder, der_len)) == NULL) {
		fprintf(stderr, "FAIL: %s d2i_ECPKParameters(Wei25519)\n", __func__);
		goto err;
	}
	if (EC_GROUP_cmp(group, new_group, ctx) != 0) {
		fprintf(stderr, "FAIL: %s Weierstrass groups do not match!\n",
		    __func__);
		goto err;
	}
#endif

	failed = 0;

 err:
	BN_CTX_end(ctx);
	BN_CTX_free(ctx);

	EC_GROUP_free(group);
	EC_GROUP_free(new_group);
	EC_POINT_free(generator);

	freezero(der, der_len);

	return failed;
}

int
main(int argc, char **argv)
{
	int failed = 0;

	failed |= ec_group_pkparameters_named_curve_test();
	failed |= ec_group_pkparameters_parameters_test();
	failed |= ec_group_pkparameters_correct_padding_test();
	failed |= ec_group_roundtrip_builtin_curves();
	failed |= ec_weierstrass25519();

	return (failed);
}
