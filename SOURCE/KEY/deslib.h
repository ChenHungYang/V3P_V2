#ifndef DESLIB_H
#define DESLIB_H

#define DES_KEY_SIZE		8		
#define DES_2LENKEY_SIZE	16		// double-length key size
#define DES_3LENKEY_SIZE	24		// triple-length key size

#define DES_BLOCK_SIZE		8

/**
 * DES encryption
 **/
extern void DES_Encrypt(unsigned char *key, 
				 unsigned char *inblock,
				 unsigned char *outblock);

/**
 * DES decryption
 **/
extern void DES_Decrypt(unsigned char *key, 
				 unsigned char *inblock,
				 unsigned char *outblock);

/**
 * Triple DES triple-length key encryption
 **/
extern void TDES_3LenKey_Encrypt(unsigned char *key, 
				 unsigned char *inblock,
				 unsigned char *outblock);

/**
 * Triple DES triple-length key decryption
 **/
extern void TDES_3LenKey_Decrypt(unsigned char *key, 
				 unsigned char *inblock,
				 unsigned char *outblock);

/**
 * Triple DES triple-length key encryption in CBC mode.
 **/
extern unsigned int TDES_3LenKey_CBC_Encrypt(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock);

/**
 * Triple DES triple-length key decryption in CBC mode.
 **/
extern unsigned int TDES_3LenKey_CBC_Decrypt(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock);

/**
 * Triple DES double-length key encryption
 **/
extern void TDES_2LenKey_Encrypt(unsigned char *key, 
				 unsigned char *inblock,
				 unsigned char *outblock);

/**
 * Triple DES double-length key decryption
 **/
extern void TDES_2LenKey_Decrypt(unsigned char *key, 
				 unsigned char *inblock,
				 unsigned char *outblock);

/**
 * Triple DES double-length key encryption in CBC mode.
 **/
extern unsigned int TDES_2LenKey_CBC_Encrypt(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock);

/**
 * AY Triple DES double-length key encryption in CBC mode with Initial Vector.
 **/
extern unsigned int TDES_2LenKey_IVCBC_Encrypt(unsigned char *key, unsigned char*initVector,
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock);


/**
 * Triple DES double-length key decryption in CBC mode.
 **/
extern unsigned int TDES_2LenKey_CBC_Decrypt(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock);

/**
 * Retail MAC in 3DES CBC mode using a double-length key.
 **/
extern void TDES_2LenKey_CBC_MAC(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock);

/**
 * Triple DES double-length key encryption in ECB mode.
 **/
unsigned int TDES_2LenKey_ECB_Encrypt(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock);

/**
 * Triple DES double-length key decryption in ECB mode.
 **/
unsigned int TDES_2LenKey_ECB_Decrypt(unsigned char *key, 
				 unsigned char *inblock,
				 int inblksize,
				 unsigned char *outblock);

#endif  /* ifndef DESLIB_H */


