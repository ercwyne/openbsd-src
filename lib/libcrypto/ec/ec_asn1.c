/* $OpenBSD: ec_asn1.c,v 1.73 2024/10/15 06:35:59 tb Exp $ */
/*
 * Written by Nils Larsch for the OpenSSL project.
 */
/* ====================================================================
 * Copyright (c) 2000-2003 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

#include <string.h>

#include <openssl/opensslconf.h>

#include <openssl/err.h>
#include <openssl/asn1t.h>
#include <openssl/objects.h>

#include "asn1_local.h"
#include "ec_local.h"

int
EC_GROUP_get_basis_type(const EC_GROUP *group)
{
	return 0;
}
LCRYPTO_ALIAS(EC_GROUP_get_basis_type);

typedef struct x9_62_pentanomial_st {
	long k1;
	long k2;
	long k3;
} X9_62_PENTANOMIAL;

typedef struct x9_62_characteristic_two_st {
	long m;
	ASN1_OBJECT *type;
	union {
		char *ptr;
		/* NID_X9_62_onBasis */
		ASN1_NULL *onBasis;
		/* NID_X9_62_tpBasis */
		ASN1_INTEGER *tpBasis;
		/* NID_X9_62_ppBasis */
		X9_62_PENTANOMIAL *ppBasis;
		/* anything else */
		ASN1_TYPE *other;
	} p;
} X9_62_CHARACTERISTIC_TWO;

typedef struct x9_62_fieldid_st {
	ASN1_OBJECT *fieldType;
	union {
		char *ptr;
		/* NID_X9_62_prime_field */
		ASN1_INTEGER *prime;
		/* NID_X9_62_characteristic_two_field */
		X9_62_CHARACTERISTIC_TWO *char_two;
		/* anything else */
		ASN1_TYPE *other;
	} p;
} X9_62_FIELDID;

typedef struct x9_62_curve_st {
	ASN1_OCTET_STRING *a;
	ASN1_OCTET_STRING *b;
	ASN1_BIT_STRING *seed;
} X9_62_CURVE;

typedef struct ec_parameters_st {
	long version;
	X9_62_FIELDID *fieldID;
	X9_62_CURVE *curve;
	ASN1_OCTET_STRING *base;
	ASN1_INTEGER *order;
	ASN1_INTEGER *cofactor;
} ECPARAMETERS;

#define ECPK_PARAM_NAMED_CURVE		0
#define ECPK_PARAM_EXPLICIT		1
#define ECPK_PARAM_IMPLICITLY_CA	2

typedef struct ecpk_parameters_st {
	int type;
	union {
		ASN1_OBJECT *named_curve;
		ECPARAMETERS *parameters;
		ASN1_NULL *implicitlyCA;
	} value;
} ECPKPARAMETERS;

typedef struct ec_privatekey_st {
	long version;
	ASN1_OCTET_STRING *privateKey;
	ECPKPARAMETERS *parameters;
	ASN1_BIT_STRING *publicKey;
} EC_PRIVATEKEY;

static const ASN1_TEMPLATE X9_62_PENTANOMIAL_seq_tt[] = {
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(X9_62_PENTANOMIAL, k1),
		.field_name = "k1",
		.item = &LONG_it,
	},
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(X9_62_PENTANOMIAL, k2),
		.field_name = "k2",
		.item = &LONG_it,
	},
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(X9_62_PENTANOMIAL, k3),
		.field_name = "k3",
		.item = &LONG_it,
	},
};

static const ASN1_ITEM X9_62_PENTANOMIAL_it = {
	.itype = ASN1_ITYPE_SEQUENCE,
	.utype = V_ASN1_SEQUENCE,
	.templates = X9_62_PENTANOMIAL_seq_tt,
	.tcount = sizeof(X9_62_PENTANOMIAL_seq_tt) / sizeof(ASN1_TEMPLATE),
	.funcs = NULL,
	.size = sizeof(X9_62_PENTANOMIAL),
	.sname = "X9_62_PENTANOMIAL",
};

static const ASN1_TEMPLATE char_two_def_tt = {
	.flags = 0,
	.tag = 0,
	.offset = offsetof(X9_62_CHARACTERISTIC_TWO, p.other),
	.field_name = "p.other",
	.item = &ASN1_ANY_it,
};

static const ASN1_ADB_TABLE X9_62_CHARACTERISTIC_TWO_adbtbl[] = {
	{
		.value = NID_X9_62_onBasis,
		.tt = {
			.flags = 0,
			.tag = 0,
			.offset = offsetof(X9_62_CHARACTERISTIC_TWO, p.onBasis),
			.field_name = "p.onBasis",
			.item = &ASN1_NULL_it,
		},
	},
	{
		.value = NID_X9_62_tpBasis,
		.tt = {
			.flags = 0,
			.tag = 0,
			.offset = offsetof(X9_62_CHARACTERISTIC_TWO, p.tpBasis),
			.field_name = "p.tpBasis",
			.item = &ASN1_INTEGER_it,
		},
	},
	{
		.value = NID_X9_62_ppBasis,
		.tt = {
			.flags = 0,
			.tag = 0,
			.offset = offsetof(X9_62_CHARACTERISTIC_TWO, p.ppBasis),
			.field_name = "p.ppBasis",
			.item = &X9_62_PENTANOMIAL_it,
		},

	},
};

static const ASN1_ADB X9_62_CHARACTERISTIC_TWO_adb = {
	.flags = 0,
	.offset = offsetof(X9_62_CHARACTERISTIC_TWO, type),
	.tbl = X9_62_CHARACTERISTIC_TWO_adbtbl,
	.tblcount = sizeof(X9_62_CHARACTERISTIC_TWO_adbtbl) / sizeof(ASN1_ADB_TABLE),
	.default_tt = &char_two_def_tt,
	.null_tt = NULL,
};

static const ASN1_TEMPLATE X9_62_CHARACTERISTIC_TWO_seq_tt[] = {
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(X9_62_CHARACTERISTIC_TWO, m),
		.field_name = "m",
		.item = &LONG_it,
	},
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(X9_62_CHARACTERISTIC_TWO, type),
		.field_name = "type",
		.item = &ASN1_OBJECT_it,
	},
	{
		.flags = ASN1_TFLG_ADB_OID,
		.tag = -1,
		.offset = 0,
		.field_name = "X9_62_CHARACTERISTIC_TWO",
		.item = (const ASN1_ITEM *)&X9_62_CHARACTERISTIC_TWO_adb,
	},
};

static const ASN1_ITEM X9_62_CHARACTERISTIC_TWO_it = {
	.itype = ASN1_ITYPE_SEQUENCE,
	.utype = V_ASN1_SEQUENCE,
	.templates = X9_62_CHARACTERISTIC_TWO_seq_tt,
	.tcount = sizeof(X9_62_CHARACTERISTIC_TWO_seq_tt) / sizeof(ASN1_TEMPLATE),
	.funcs = NULL,
	.size = sizeof(X9_62_CHARACTERISTIC_TWO),
	.sname = "X9_62_CHARACTERISTIC_TWO",
};

static const ASN1_TEMPLATE fieldID_def_tt = {
	.flags = 0,
	.tag = 0,
	.offset = offsetof(X9_62_FIELDID, p.other),
	.field_name = "p.other",
	.item = &ASN1_ANY_it,
};

static const ASN1_ADB_TABLE X9_62_FIELDID_adbtbl[] = {
	{
		.value = NID_X9_62_prime_field,
		.tt = {
			.flags = 0,
			.tag = 0,
			.offset = offsetof(X9_62_FIELDID, p.prime),
			.field_name = "p.prime",
			.item = &ASN1_INTEGER_it,
		},
	},
	{
		.value = NID_X9_62_characteristic_two_field,
		.tt = {
			.flags = 0,
			.tag = 0,
			.offset = offsetof(X9_62_FIELDID, p.char_two),
			.field_name = "p.char_two",
			.item = &X9_62_CHARACTERISTIC_TWO_it,
		},
	},
};

static const ASN1_ADB X9_62_FIELDID_adb = {
	.flags = 0,
	.offset = offsetof(X9_62_FIELDID, fieldType),
	.tbl = X9_62_FIELDID_adbtbl,
	.tblcount = sizeof(X9_62_FIELDID_adbtbl) / sizeof(ASN1_ADB_TABLE),
	.default_tt = &fieldID_def_tt,
	.null_tt = NULL,
};

static const ASN1_TEMPLATE X9_62_FIELDID_seq_tt[] = {
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(X9_62_FIELDID, fieldType),
		.field_name = "fieldType",
		.item = &ASN1_OBJECT_it,
	},
	{
		.flags = ASN1_TFLG_ADB_OID,
		.tag = -1,
		.offset = 0,
		.field_name = "X9_62_FIELDID",
		.item = (const ASN1_ITEM *)&X9_62_FIELDID_adb,
	},
};

static const ASN1_ITEM X9_62_FIELDID_it = {
	.itype = ASN1_ITYPE_SEQUENCE,
	.utype = V_ASN1_SEQUENCE,
	.templates = X9_62_FIELDID_seq_tt,
	.tcount = sizeof(X9_62_FIELDID_seq_tt) / sizeof(ASN1_TEMPLATE),
	.funcs = NULL,
	.size = sizeof(X9_62_FIELDID),
	.sname = "X9_62_FIELDID",
};

static const ASN1_TEMPLATE X9_62_CURVE_seq_tt[] = {
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(X9_62_CURVE, a),
		.field_name = "a",
		.item = &ASN1_OCTET_STRING_it,
	},
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(X9_62_CURVE, b),
		.field_name = "b",
		.item = &ASN1_OCTET_STRING_it,
	},
	{
		.flags = ASN1_TFLG_OPTIONAL,
		.tag = 0,
		.offset = offsetof(X9_62_CURVE, seed),
		.field_name = "seed",
		.item = &ASN1_BIT_STRING_it,
	},
};

static const ASN1_ITEM X9_62_CURVE_it = {
	.itype = ASN1_ITYPE_SEQUENCE,
	.utype = V_ASN1_SEQUENCE,
	.templates = X9_62_CURVE_seq_tt,
	.tcount = sizeof(X9_62_CURVE_seq_tt) / sizeof(ASN1_TEMPLATE),
	.funcs = NULL,
	.size = sizeof(X9_62_CURVE),
	.sname = "X9_62_CURVE",
};

static const ASN1_TEMPLATE ECPARAMETERS_seq_tt[] = {
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(ECPARAMETERS, version),
		.field_name = "version",
		.item = &LONG_it,
	},
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(ECPARAMETERS, fieldID),
		.field_name = "fieldID",
		.item = &X9_62_FIELDID_it,
	},
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(ECPARAMETERS, curve),
		.field_name = "curve",
		.item = &X9_62_CURVE_it,
	},
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(ECPARAMETERS, base),
		.field_name = "base",
		.item = &ASN1_OCTET_STRING_it,
	},
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(ECPARAMETERS, order),
		.field_name = "order",
		.item = &ASN1_INTEGER_it,
	},
	{
		.flags = ASN1_TFLG_OPTIONAL,
		.tag = 0,
		.offset = offsetof(ECPARAMETERS, cofactor),
		.field_name = "cofactor",
		.item = &ASN1_INTEGER_it,
	},
};

static const ASN1_ITEM ECPARAMETERS_it = {
	.itype = ASN1_ITYPE_SEQUENCE,
	.utype = V_ASN1_SEQUENCE,
	.templates = ECPARAMETERS_seq_tt,
	.tcount = sizeof(ECPARAMETERS_seq_tt) / sizeof(ASN1_TEMPLATE),
	.funcs = NULL,
	.size = sizeof(ECPARAMETERS),
	.sname = "ECPARAMETERS",
};

static ECPARAMETERS *
ECPARAMETERS_new(void)
{
	return (ECPARAMETERS*)ASN1_item_new(&ECPARAMETERS_it);
}

static void
ECPARAMETERS_free(ECPARAMETERS *a)
{
	ASN1_item_free((ASN1_VALUE *)a, &ECPARAMETERS_it);
}

static const ASN1_TEMPLATE ECPKPARAMETERS_ch_tt[] = {
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(ECPKPARAMETERS, value.named_curve),
		.field_name = "value.named_curve",
		.item = &ASN1_OBJECT_it,
	},
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(ECPKPARAMETERS, value.parameters),
		.field_name = "value.parameters",
		.item = &ECPARAMETERS_it,
	},
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(ECPKPARAMETERS, value.implicitlyCA),
		.field_name = "value.implicitlyCA",
		.item = &ASN1_NULL_it,
	},
};

static const ASN1_ITEM ECPKPARAMETERS_it = {
	.itype = ASN1_ITYPE_CHOICE,
	.utype = offsetof(ECPKPARAMETERS, type),
	.templates = ECPKPARAMETERS_ch_tt,
	.tcount = sizeof(ECPKPARAMETERS_ch_tt) / sizeof(ASN1_TEMPLATE),
	.funcs = NULL,
	.size = sizeof(ECPKPARAMETERS),
	.sname = "ECPKPARAMETERS",
};

static ECPKPARAMETERS *
d2i_ECPKPARAMETERS(ECPKPARAMETERS **a, const unsigned char **in, long len)
{
	return (ECPKPARAMETERS *)ASN1_item_d2i((ASN1_VALUE **)a, in, len,
	    &ECPKPARAMETERS_it);
}

static int
i2d_ECPKPARAMETERS(const ECPKPARAMETERS *a, unsigned char **out)
{
	return ASN1_item_i2d((ASN1_VALUE *)a, out, &ECPKPARAMETERS_it);
}

static ECPKPARAMETERS *
ECPKPARAMETERS_new(void)
{
	return (ECPKPARAMETERS *)ASN1_item_new(&ECPKPARAMETERS_it);
}

static void
ECPKPARAMETERS_free(ECPKPARAMETERS *a)
{
	ASN1_item_free((ASN1_VALUE *)a, &ECPKPARAMETERS_it);
}

static const ASN1_TEMPLATE EC_PRIVATEKEY_seq_tt[] = {
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(EC_PRIVATEKEY, version),
		.field_name = "version",
		.item = &LONG_it,
	},
	{
		.flags = 0,
		.tag = 0,
		.offset = offsetof(EC_PRIVATEKEY, privateKey),
		.field_name = "privateKey",
		.item = &ASN1_OCTET_STRING_it,
	},
	{
		.flags = ASN1_TFLG_EXPLICIT | ASN1_TFLG_OPTIONAL,
		.tag = 0,
		.offset = offsetof(EC_PRIVATEKEY, parameters),
		.field_name = "parameters",
		.item = &ECPKPARAMETERS_it,
	},
	{
		.flags = ASN1_TFLG_EXPLICIT | ASN1_TFLG_OPTIONAL,
		.tag = 1,
		.offset = offsetof(EC_PRIVATEKEY, publicKey),
		.field_name = "publicKey",
		.item = &ASN1_BIT_STRING_it,
	},
};

static const ASN1_ITEM EC_PRIVATEKEY_it = {
	.itype = ASN1_ITYPE_SEQUENCE,
	.utype = V_ASN1_SEQUENCE,
	.templates = EC_PRIVATEKEY_seq_tt,
	.tcount = sizeof(EC_PRIVATEKEY_seq_tt) / sizeof(ASN1_TEMPLATE),
	.funcs = NULL,
	.size = sizeof(EC_PRIVATEKEY),
	.sname = "EC_PRIVATEKEY",
};

static EC_PRIVATEKEY *
d2i_EC_PRIVATEKEY(EC_PRIVATEKEY **a, const unsigned char **in, long len)
{
	return (EC_PRIVATEKEY *)ASN1_item_d2i((ASN1_VALUE **)a, in, len,
	    &EC_PRIVATEKEY_it);
}

static int
i2d_EC_PRIVATEKEY(const EC_PRIVATEKEY *a, unsigned char **out)
{
	return ASN1_item_i2d((ASN1_VALUE *)a, out, &EC_PRIVATEKEY_it);
}

static EC_PRIVATEKEY *
EC_PRIVATEKEY_new(void)
{
	return (EC_PRIVATEKEY *)ASN1_item_new(&EC_PRIVATEKEY_it);
}

static void
EC_PRIVATEKEY_free(EC_PRIVATEKEY *a)
{
	ASN1_item_free((ASN1_VALUE *)a, &EC_PRIVATEKEY_it);
}

static int
ec_asn1_group2fieldid(const EC_GROUP *group, X9_62_FIELDID *field)
{
	BIGNUM *p = NULL;
	int nid;
	int ret = 0;

	if (group == NULL || field == NULL)
		goto err;

	nid = EC_METHOD_get_field_type(EC_GROUP_method_of(group));
	if (nid == NID_X9_62_characteristic_two_field) {
		ECerror(EC_R_GF2M_NOT_SUPPORTED);
		goto err;
	}
	if (nid != NID_X9_62_prime_field) {
		ECerror(EC_R_INVALID_FIELD);
		goto err;
	}

	if ((field->fieldType = OBJ_nid2obj(nid)) == NULL) {
		ECerror(ERR_R_OBJ_LIB);
		goto err;
	}
	if ((p = BN_new()) == NULL) {
		ECerror(ERR_R_MALLOC_FAILURE);
		goto err;
	}
	if (!EC_GROUP_get_curve(group, p, NULL, NULL, NULL)) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	if ((field->p.prime = BN_to_ASN1_INTEGER(p, NULL)) == NULL) {
		ECerror(ERR_R_ASN1_LIB);
		goto err;
	}

	ret = 1;

 err:
	BN_free(p);

	return ret;
}

static int
ec_asn1_encode_field_element(const EC_GROUP *group, const BIGNUM *bn,
    ASN1_OCTET_STRING *os)
{
	unsigned char *buf;
	int len;
	int ret = 0;

	/* Zero-pad field element per SEC 1, section 2.3.5. */
	len = (EC_GROUP_get_degree(group) + 7) / 8;

	/* One extra byte for historic NUL termination of ASN1_STRINGs. */
	if ((buf = calloc(1, len + 1)) == NULL)
		goto err;

	if (BN_bn2binpad(bn, buf, len) != len)
		goto err;

	ASN1_STRING_set0(os, buf, len);
	buf = NULL;
	len = 0;

	ret = 1;

 err:
	freezero(buf, len);

	return ret;
}

static int
ec_asn1_group2curve(const EC_GROUP *group, X9_62_CURVE *curve)
{
	BIGNUM *a = NULL, *b = NULL;
	int ret = 0;

	if (group == NULL)
		goto err;
	if (curve == NULL || curve->a == NULL || curve->b == NULL)
		goto err;

	if ((a = BN_new()) == NULL || (b = BN_new()) == NULL) {
		ECerror(ERR_R_MALLOC_FAILURE);
		goto err;
	}

	if (!EC_GROUP_get_curve(group, NULL, a, b, NULL)) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}

	if (!ec_asn1_encode_field_element(group, a, curve->a)) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	if (!ec_asn1_encode_field_element(group, b, curve->b)) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}

	ASN1_BIT_STRING_free(curve->seed);
	curve->seed = NULL;

	if (group->seed != NULL) {
		if ((curve->seed = ASN1_BIT_STRING_new()) == NULL) {
			ECerror(ERR_R_MALLOC_FAILURE);
			goto err;
		}
		if (!ASN1_BIT_STRING_set(curve->seed,
		    group->seed, group->seed_len)) {
			ECerror(ERR_R_ASN1_LIB);
			goto err;
		}
		if (!asn1_abs_set_unused_bits(curve->seed, 0)) {
			ECerror(ERR_R_ASN1_LIB);
			goto err;
		}
	}

	ret = 1;

 err:
	BN_free(a);
	BN_free(b);

	return ret;
}

static ECPARAMETERS *
ec_asn1_group2parameters(const EC_GROUP *group)
{
	int ok = 0;
	size_t len = 0;
	ECPARAMETERS *ret = NULL;
	const BIGNUM *order, *cofactor;
	unsigned char *buffer = NULL;
	const EC_POINT *point = NULL;
	point_conversion_form_t form;

	if ((ret = ECPARAMETERS_new()) == NULL) {
		ECerror(ERR_R_MALLOC_FAILURE);
		goto err;
	}

	/* set the version (always one) */
	ret->version = (long) 0x1;

	/* set the fieldID */
	if (!ec_asn1_group2fieldid(group, ret->fieldID)) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	/* set the curve */
	if (!ec_asn1_group2curve(group, ret->curve)) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	/* set the base point */
	if ((point = EC_GROUP_get0_generator(group)) == NULL) {
		ECerror(EC_R_UNDEFINED_GENERATOR);
		goto err;
	}
	form = EC_GROUP_get_point_conversion_form(group);

	len = EC_POINT_point2oct(group, point, form, NULL, len, NULL);
	if (len == 0) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	if ((buffer = malloc(len)) == NULL) {
		ECerror(ERR_R_MALLOC_FAILURE);
		goto err;
	}
	if (!EC_POINT_point2oct(group, point, form, buffer, len, NULL)) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	if (ret->base == NULL && (ret->base = ASN1_OCTET_STRING_new()) == NULL) {
		ECerror(ERR_R_MALLOC_FAILURE);
		goto err;
	}
	if (!ASN1_OCTET_STRING_set(ret->base, buffer, len)) {
		ECerror(ERR_R_ASN1_LIB);
		goto err;
	}
	if ((order = EC_GROUP_get0_order(group)) == NULL) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	if (BN_is_zero(order)) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	ASN1_INTEGER_free(ret->order);
	if ((ret->order = BN_to_ASN1_INTEGER(order, NULL)) == NULL) {
		ECerror(ERR_R_ASN1_LIB);
		goto err;
	}
	ASN1_INTEGER_free(ret->cofactor);
	ret->cofactor = NULL;
	if ((cofactor = EC_GROUP_get0_cofactor(group)) == NULL) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	if (!BN_is_zero(cofactor)) {
		if ((ret->cofactor = BN_to_ASN1_INTEGER(cofactor, NULL)) == NULL) {
			ECerror(ERR_R_ASN1_LIB);
			goto err;
		}
	}
	ok = 1;

 err:
	if (!ok) {
		ECPARAMETERS_free(ret);
		ret = NULL;
	}
	free(buffer);
	return (ret);
}

ECPKPARAMETERS *
ec_asn1_group2pkparameters(const EC_GROUP *group)
{
	ECPKPARAMETERS *pkparameters;
	ECPARAMETERS *parameters;
	ASN1_OBJECT *aobj;
	int nid;

	if ((pkparameters = ECPKPARAMETERS_new()) == NULL) {
		ECerror(ERR_R_MALLOC_FAILURE);
		goto err;
	}

	if ((EC_GROUP_get_asn1_flag(group) & OPENSSL_EC_NAMED_CURVE) != 0) {
		if ((nid = EC_GROUP_get_curve_name(group)) == NID_undef)
			goto err;
		if ((aobj = OBJ_nid2obj(nid)) == NULL)
			goto err;
		pkparameters->type = ECPK_PARAM_NAMED_CURVE;
		pkparameters->value.named_curve = aobj;
	} else {
		if ((parameters = ec_asn1_group2parameters(group)) == NULL)
			goto err;
		pkparameters->type = ECPK_PARAM_EXPLICIT;
		pkparameters->value.parameters = parameters;
		parameters = NULL;
	}

	return pkparameters;

 err:
	ECPKPARAMETERS_free(pkparameters);

	return NULL;
}

static EC_GROUP *
ec_asn1_parameters2group(const ECPARAMETERS *params)
{
	int ok = 0, tmp;
	EC_GROUP *ret = NULL;
	BIGNUM *p = NULL, *a = NULL, *b = NULL, *order = NULL, *cofactor = NULL;
	EC_POINT *point = NULL;
	int field_bits;

	if (!params->fieldID || !params->fieldID->fieldType ||
	    !params->fieldID->p.ptr) {
		ECerror(EC_R_ASN1_ERROR);
		goto err;
	}
	/* now extract the curve parameters a and b */
	if (!params->curve || !params->curve->a ||
	    !params->curve->a->data || !params->curve->b ||
	    !params->curve->b->data) {
		ECerror(EC_R_ASN1_ERROR);
		goto err;
	}
	a = BN_bin2bn(params->curve->a->data, params->curve->a->length, NULL);
	if (a == NULL) {
		ECerror(ERR_R_BN_LIB);
		goto err;
	}
	b = BN_bin2bn(params->curve->b->data, params->curve->b->length, NULL);
	if (b == NULL) {
		ECerror(ERR_R_BN_LIB);
		goto err;
	}
	/* get the field parameters */
	tmp = OBJ_obj2nid(params->fieldID->fieldType);
	if (tmp == NID_X9_62_characteristic_two_field) {
		ECerror(EC_R_GF2M_NOT_SUPPORTED);
		goto err;
	} else if (tmp == NID_X9_62_prime_field) {
		/* we have a curve over a prime field */
		/* extract the prime number */
		if (!params->fieldID->p.prime) {
			ECerror(EC_R_ASN1_ERROR);
			goto err;
		}
		p = ASN1_INTEGER_to_BN(params->fieldID->p.prime, NULL);
		if (p == NULL) {
			ECerror(ERR_R_ASN1_LIB);
			goto err;
		}
		if (BN_is_negative(p) || BN_is_zero(p)) {
			ECerror(EC_R_INVALID_FIELD);
			goto err;
		}
		field_bits = BN_num_bits(p);
		if (field_bits > OPENSSL_ECC_MAX_FIELD_BITS) {
			ECerror(EC_R_FIELD_TOO_LARGE);
			goto err;
		}
		/* create the EC_GROUP structure */
		ret = EC_GROUP_new_curve_GFp(p, a, b, NULL);
	} else {
		ECerror(EC_R_INVALID_FIELD);
		goto err;
	}

	if (ret == NULL) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	/* extract seed (optional) */
	if (params->curve->seed != NULL) {
		free(ret->seed);
		if (!(ret->seed = malloc(params->curve->seed->length))) {
			ECerror(ERR_R_MALLOC_FAILURE);
			goto err;
		}
		memcpy(ret->seed, params->curve->seed->data,
		    params->curve->seed->length);
		ret->seed_len = params->curve->seed->length;
	}
	if (!params->order || !params->base || !params->base->data) {
		ECerror(EC_R_ASN1_ERROR);
		goto err;
	}
	if ((point = EC_POINT_new(ret)) == NULL)
		goto err;

	/* set the point conversion form */
	EC_GROUP_set_point_conversion_form(ret, (point_conversion_form_t)
	    (params->base->data[0] & ~0x01));

	/* extract the ec point */
	if (!EC_POINT_oct2point(ret, point, params->base->data,
		params->base->length, NULL)) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	if ((order = ASN1_INTEGER_to_BN(params->order, NULL)) == NULL) {
		ECerror(ERR_R_ASN1_LIB);
		goto err;
	}
	if (BN_is_negative(order) || BN_is_zero(order)) {
		ECerror(EC_R_INVALID_GROUP_ORDER);
		goto err;
	}
	if (BN_num_bits(order) > field_bits + 1) {	/* Hasse bound */
		ECerror(EC_R_INVALID_GROUP_ORDER);
		goto err;
	}
	if (params->cofactor != NULL) {
		if ((cofactor = ASN1_INTEGER_to_BN(params->cofactor,
		    NULL)) == NULL) {
			ECerror(ERR_R_ASN1_LIB);
			goto err;
		}
	}
	if (!EC_GROUP_set_generator(ret, point, order, cofactor)) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	ok = 1;

 err:
	if (!ok) {
		EC_GROUP_free(ret);
		ret = NULL;
	}
	BN_free(p);
	BN_free(a);
	BN_free(b);
	BN_free(order);
	BN_free(cofactor);
	EC_POINT_free(point);

	return ret;
}

EC_GROUP *
ec_asn1_pkparameters2group(const ECPKPARAMETERS *params)
{
	EC_GROUP *group;
	int nid;

	if (params->type == ECPK_PARAM_NAMED_CURVE) {
		if ((nid = OBJ_obj2nid(params->value.named_curve)) == NID_undef) {
			ECerror(EC_R_UNKNOWN_GROUP);
			return NULL;
		}
		if ((group = EC_GROUP_new_by_curve_name(nid)) == NULL) {
			ECerror(EC_R_EC_GROUP_NEW_BY_NAME_FAILURE);
			return NULL;
		}
		EC_GROUP_set_asn1_flag(group, OPENSSL_EC_NAMED_CURVE);
	} else if (params->type == ECPK_PARAM_EXPLICIT) {
		group = ec_asn1_parameters2group(params->value.parameters);
		if (group == NULL) {
			ECerror(ERR_R_EC_LIB);
			return NULL;
		}
		EC_GROUP_set_asn1_flag(group, 0);
	} else if (params->type == ECPK_PARAM_IMPLICITLY_CA) {
		return NULL;
	} else {
		ECerror(EC_R_ASN1_ERROR);
		return NULL;
	}

	return group;
}

EC_GROUP *
d2i_ECPKParameters(EC_GROUP **a, const unsigned char **in, long len)
{
	EC_GROUP *group = NULL;
	ECPKPARAMETERS *params;

	if ((params = d2i_ECPKPARAMETERS(NULL, in, len)) == NULL) {
		ECerror(EC_R_D2I_ECPKPARAMETERS_FAILURE);
		goto err;
	}
	if ((group = ec_asn1_pkparameters2group(params)) == NULL) {
		ECerror(EC_R_PKPARAMETERS2GROUP_FAILURE);
		goto err;
	}

	if (a != NULL) {
		EC_GROUP_free(*a);
		*a = group;
	}

 err:
	ECPKPARAMETERS_free(params);
	return (group);
}
LCRYPTO_ALIAS(d2i_ECPKParameters);

int
i2d_ECPKParameters(const EC_GROUP *group, unsigned char **out_der)
{
	ECPKPARAMETERS *parameters;
	int ret = 0;

	if ((parameters = ec_asn1_group2pkparameters(group)) == NULL) {
		ECerror(EC_R_GROUP2PKPARAMETERS_FAILURE);
		goto err;
	}
	if ((ret = i2d_ECPKPARAMETERS(parameters, out_der)) <= 0) {
		ECerror(EC_R_I2D_ECPKPARAMETERS_FAILURE);
		goto err;
	}

 err:
	ECPKPARAMETERS_free(parameters);

	return ret;
}
LCRYPTO_ALIAS(i2d_ECPKParameters);

EC_KEY *
d2i_ECPrivateKey(EC_KEY **a, const unsigned char **in, long len)
{
	EC_KEY *ret = NULL;
	EC_PRIVATEKEY *priv_key = NULL;

	if ((priv_key = d2i_EC_PRIVATEKEY(NULL, in, len)) == NULL) {
		ECerror(ERR_R_EC_LIB);
		return NULL;
	}
	if (a == NULL || *a == NULL) {
		if ((ret = EC_KEY_new()) == NULL) {
			ECerror(ERR_R_MALLOC_FAILURE);
			goto err;
		}
	} else
		ret = *a;

	if (priv_key->parameters) {
		EC_GROUP_free(ret->group);
		ret->group = ec_asn1_pkparameters2group(priv_key->parameters);
	}
	if (ret->group == NULL) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	ret->version = priv_key->version;

	if (priv_key->privateKey) {
		ret->priv_key = BN_bin2bn(
		    ASN1_STRING_data(priv_key->privateKey),
		    ASN1_STRING_length(priv_key->privateKey),
		    ret->priv_key);
		if (ret->priv_key == NULL) {
			ECerror(ERR_R_BN_LIB);
			goto err;
		}
	} else {
		ECerror(EC_R_MISSING_PRIVATE_KEY);
		goto err;
	}

	if (ret->pub_key)
		EC_POINT_free(ret->pub_key);
	ret->pub_key = EC_POINT_new(ret->group);
	if (ret->pub_key == NULL) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}

	if (priv_key->publicKey) {
		const unsigned char *pub_oct;
		size_t pub_oct_len;

		pub_oct = ASN1_STRING_data(priv_key->publicKey);
		pub_oct_len = ASN1_STRING_length(priv_key->publicKey);
		if (pub_oct == NULL || pub_oct_len <= 0) {
			ECerror(EC_R_BUFFER_TOO_SMALL);
			goto err;
		}

		/* save the point conversion form */
		ret->conv_form = (point_conversion_form_t) (pub_oct[0] & ~0x01);
		if (!EC_POINT_oct2point(ret->group, ret->pub_key,
			pub_oct, pub_oct_len, NULL)) {
			ECerror(ERR_R_EC_LIB);
			goto err;
		}
	} else {
		if (!EC_POINT_mul(ret->group, ret->pub_key, ret->priv_key,
			NULL, NULL, NULL)) {
			ECerror(ERR_R_EC_LIB);
			goto err;
		}
		/* Remember the original private-key-only encoding. */
		ret->enc_flag |= EC_PKEY_NO_PUBKEY;
	}

	EC_PRIVATEKEY_free(priv_key);
	if (a != NULL)
		*a = ret;
	return (ret);

 err:
	if (a == NULL || *a != ret)
		EC_KEY_free(ret);
	if (priv_key)
		EC_PRIVATEKEY_free(priv_key);

	return (NULL);
}
LCRYPTO_ALIAS(d2i_ECPrivateKey);

int
i2d_ECPrivateKey(EC_KEY *a, unsigned char **out)
{
	int ret = 0, ok = 0;
	unsigned char *buffer = NULL;
	size_t buf_len = 0, tmp_len;
	EC_PRIVATEKEY *priv_key = NULL;

	if (a == NULL || a->group == NULL || a->priv_key == NULL ||
	    (!(a->enc_flag & EC_PKEY_NO_PUBKEY) && a->pub_key == NULL)) {
		ECerror(ERR_R_PASSED_NULL_PARAMETER);
		goto err;
	}
	if ((priv_key = EC_PRIVATEKEY_new()) == NULL) {
		ECerror(ERR_R_MALLOC_FAILURE);
		goto err;
	}
	priv_key->version = a->version;

	buf_len = (size_t) BN_num_bytes(a->priv_key);
	buffer = malloc(buf_len);
	if (buffer == NULL) {
		ECerror(ERR_R_MALLOC_FAILURE);
		goto err;
	}
	if (!BN_bn2bin(a->priv_key, buffer)) {
		ECerror(ERR_R_BN_LIB);
		goto err;
	}
	if (!ASN1_STRING_set(priv_key->privateKey, buffer, buf_len)) {
		ECerror(ERR_R_ASN1_LIB);
		goto err;
	}
	if (!(a->enc_flag & EC_PKEY_NO_PARAMETERS)) {
		ECPKPARAMETERS *parameters;

		if ((parameters = ec_asn1_group2pkparameters(a->group)) == NULL) {
			ECerror(ERR_R_EC_LIB);
			goto err;
		}
		priv_key->parameters = parameters;
	}
	if (!(a->enc_flag & EC_PKEY_NO_PUBKEY) && a->pub_key != NULL) {
		priv_key->publicKey = ASN1_BIT_STRING_new();
		if (priv_key->publicKey == NULL) {
			ECerror(ERR_R_MALLOC_FAILURE);
			goto err;
		}
		tmp_len = EC_POINT_point2oct(a->group, a->pub_key,
		    a->conv_form, NULL, 0, NULL);

		if (tmp_len > buf_len) {
			unsigned char *tmp_buffer = realloc(buffer, tmp_len);
			if (!tmp_buffer) {
				ECerror(ERR_R_MALLOC_FAILURE);
				goto err;
			}
			buffer = tmp_buffer;
			buf_len = tmp_len;
		}
		if (!EC_POINT_point2oct(a->group, a->pub_key,
			a->conv_form, buffer, buf_len, NULL)) {
			ECerror(ERR_R_EC_LIB);
			goto err;
		}
		if (!ASN1_STRING_set(priv_key->publicKey, buffer, buf_len)) {
			ECerror(ERR_R_ASN1_LIB);
			goto err;
		}
		if (!asn1_abs_set_unused_bits(priv_key->publicKey, 0)) {
			ECerror(ERR_R_ASN1_LIB);
			goto err;
		}
	}
	if ((ret = i2d_EC_PRIVATEKEY(priv_key, out)) == 0) {
		ECerror(ERR_R_EC_LIB);
		goto err;
	}
	ok = 1;
 err:
	free(buffer);
	if (priv_key)
		EC_PRIVATEKEY_free(priv_key);
	return (ok ? ret : 0);
}
LCRYPTO_ALIAS(i2d_ECPrivateKey);

int
i2d_ECParameters(EC_KEY *a, unsigned char **out)
{
	if (a == NULL) {
		ECerror(ERR_R_PASSED_NULL_PARAMETER);
		return 0;
	}
	return i2d_ECPKParameters(a->group, out);
}
LCRYPTO_ALIAS(i2d_ECParameters);

EC_KEY *
d2i_ECParameters(EC_KEY **a, const unsigned char **in, long len)
{
	EC_KEY *ret;

	if (in == NULL || *in == NULL) {
		ECerror(ERR_R_PASSED_NULL_PARAMETER);
		return NULL;
	}
	if (a == NULL || *a == NULL) {
		if ((ret = EC_KEY_new()) == NULL) {
			ECerror(ERR_R_MALLOC_FAILURE);
			return NULL;
		}
	} else
		ret = *a;

	if (!d2i_ECPKParameters(&ret->group, in, len)) {
		ECerror(ERR_R_EC_LIB);
		if (a == NULL || *a != ret)
			EC_KEY_free(ret);
		return NULL;
	}

	if (a != NULL)
		*a = ret;
	return ret;
}
LCRYPTO_ALIAS(d2i_ECParameters);

EC_KEY *
o2i_ECPublicKey(EC_KEY **a, const unsigned char **in, long len)
{
	EC_KEY *ret = NULL;

	if (a == NULL || (*a) == NULL || (*a)->group == NULL) {
		/* An EC_GROUP structure is necessary to set the public key. */
		ECerror(ERR_R_PASSED_NULL_PARAMETER);
		return 0;
	}
	ret = *a;
	if (ret->pub_key == NULL &&
	    (ret->pub_key = EC_POINT_new(ret->group)) == NULL) {
		ECerror(ERR_R_MALLOC_FAILURE);
		return 0;
	}
	if (!EC_POINT_oct2point(ret->group, ret->pub_key, *in, len, NULL)) {
		ECerror(ERR_R_EC_LIB);
		return 0;
	}
	/* save the point conversion form */
	ret->conv_form = (point_conversion_form_t) (*in[0] & ~0x01);
	*in += len;
	return ret;
}
LCRYPTO_ALIAS(o2i_ECPublicKey);

int
i2o_ECPublicKey(const EC_KEY *a, unsigned char **out)
{
	size_t buf_len = 0;
	int new_buffer = 0;

	if (a == NULL) {
		ECerror(ERR_R_PASSED_NULL_PARAMETER);
		return 0;
	}
	buf_len = EC_POINT_point2oct(a->group, a->pub_key,
	    a->conv_form, NULL, 0, NULL);

	if (out == NULL || buf_len == 0)
		/* out == NULL => just return the length of the octet string */
		return buf_len;

	if (*out == NULL) {
		if ((*out = malloc(buf_len)) == NULL) {
			ECerror(ERR_R_MALLOC_FAILURE);
			return 0;
		}
		new_buffer = 1;
	}
	if (!EC_POINT_point2oct(a->group, a->pub_key, a->conv_form,
		*out, buf_len, NULL)) {
		ECerror(ERR_R_EC_LIB);
		if (new_buffer) {
			free(*out);
			*out = NULL;
		}
		return 0;
	}
	if (!new_buffer)
		*out += buf_len;
	return buf_len;
}
LCRYPTO_ALIAS(i2o_ECPublicKey);
