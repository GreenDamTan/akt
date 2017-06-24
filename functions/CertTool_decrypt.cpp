#include "CertTool_decrypt.h"

unsigned long CT_a;

unsigned long CT_mult(long p, long q)
{
    unsigned long p1 = p / 10000L, p0 = p % 10000L, q1 = q / 10000L, q0 = q % 10000L;
    return (((p0 * q1 + p1 * q0) % 10000L) * 10000L + p0 * q0) % 100000000L;
}

unsigned long CT_NextRandomRange(long range)
{
    CT_a = (CT_mult(CT_a, 31415821L) + 1) % 100000000L;
    return (((CT_a / 10000L) * range) / 10000L);
}

unsigned char* CT_GetCryptBytes(unsigned int seed, unsigned int size)
{
    CT_a = seed;
    unsigned char* arry = (unsigned char*)malloc2(size + 4);
    memset(arry, 0, size + 4);
    for(unsigned int x = 0; x < size + 4; x++)
        arry[x] = (unsigned char)(CT_NextRandomRange(256) & 0xFF);
    return arry + 4; //Skip first 4 bytes
}

unsigned char* CT_Decrypt(unsigned char** data, unsigned char** rand, unsigned int size)
{
    if(!size)
        return data[0];
    for(unsigned int i = 0; i < size; i++)
        data[0][i] ^= rand[0][i];
    data[0] += size;
    rand[0] += size;
    return data[0] - size;
}

void CT_DecryptCerts()
{
    CERT_DATA* cd = CT_cert_data;

    if(!cd->raw_data || !cd->raw_size)
        return;

    unsigned int real_cert_size = FindBAADF00DPattern(cd->raw_data, cd->raw_size);
    if(!real_cert_size)
        real_cert_size = cd->raw_size;
    unsigned char* rand = CT_GetCryptBytes(cd->decrypt_seed[0], real_cert_size);
    unsigned char* rand_start = rand;
    unsigned char* decr = (unsigned char*)malloc2(real_cert_size);
    unsigned char* decr_start = decr;
    memcpy(decr, cd->raw_data, real_cert_size);
    free2(cd->raw_data);
    memcpy(&cd->first_dw, decr, sizeof(unsigned int));
    CT_Decrypt(&decr, &rand, 4 * sizeof(unsigned int));
    decr += 2 + 4; //Skipped bytes

    cd->projectid_diff = decr - decr_start + sizeof(unsigned short); //pointer + word for size, 0x18
    CT_a = cd->decrypt_seed[0];
    for(unsigned int i = 0; i < cd->projectid_diff; i++)
        CT_NextRandomRange(256);
    cd->decrypt_seed[1] = CT_a;

    //Project ID
    unsigned short* projectID_size = (unsigned short*)CT_Decrypt(&decr, &rand, sizeof(unsigned short));
    if(*projectID_size)
    {
        cd->projectid = (char*)malloc2(*projectID_size + 1);
        memset(cd->projectid, 0, *projectID_size + 1);
        memcpy(cd->projectid, CT_Decrypt(&decr, &rand, *projectID_size), *projectID_size);
    }
    //Customer Service
    unsigned short* customerSER_size = (unsigned short*)CT_Decrypt(&decr, &rand, sizeof(unsigned short));
    if(*customerSER_size)
    {
        cd->customer_service = (char*)malloc2(*customerSER_size + 1);
        memset(cd->customer_service, 0, *customerSER_size + 1);
        memcpy(cd->customer_service, CT_Decrypt(&decr, &rand, *customerSER_size), *customerSER_size);
    }
    //Website
    unsigned short* website_size = (unsigned short*)CT_Decrypt(&decr, &rand, sizeof(unsigned short));
    if(*website_size)
    {
        cd->website = (char*)malloc2(*website_size + 1);
        memset(cd->website, 0, *website_size + 1);
        memcpy(cd->website, CT_Decrypt(&decr, &rand, *website_size), *website_size);
    }

    decr += cd->decrypt_addvals[0]; //add the first seed

    //Stolen Codes KeyBytes
    cd->stolen_keys_diff = decr - decr_start;
    CT_a = cd->decrypt_seed[0];
    for(unsigned int i = 0; i < cd->stolen_keys_diff; i++)
        CT_NextRandomRange(256);
    cd->decrypt_seed[3] = CT_a;
    unsigned char* stolen_keys = decr;
    unsigned char* stolen_size = CT_Decrypt(&decr, &rand, sizeof(unsigned char));
    while(*stolen_size)
    {
        CT_Decrypt(&decr, &rand, *stolen_size);
        stolen_size = CT_Decrypt(&decr, &rand, sizeof(unsigned char));
    }
    unsigned int total_stolen_size = 0;
    total_stolen_size = decr - stolen_keys;
    if(total_stolen_size - 1) //minus standard 00 byte
    {
        cd->stolen_keys_size = total_stolen_size;
        cd->stolen_keys = stolen_keys;
    }
    decr += cd->decrypt_addvals[1]; //add second seed

    //Intercepted libraries
    unsigned short* libs_size = (unsigned short*)CT_Decrypt(&decr, &rand, 2);
    if(*libs_size)
    {
        cd->intercepted_libs_size = *libs_size;
        cd->intercepted_libs = (unsigned char*)malloc2(*libs_size);
        memset(cd->intercepted_libs, 0, *libs_size);
        memcpy(cd->intercepted_libs, CT_Decrypt(&decr, &rand, *libs_size), *libs_size);
    }
    decr += cd->decrypt_addvals[2]; //add third seed

    //Certificates
    unsigned char* decr_cert = decr;
    cd->initial_diff = decr - decr_start;
    unsigned int real_size = 0;

    //Get certificate initial seed
    unsigned int seed_count = rand - rand_start;
    CT_a = cd->decrypt_seed[0];
    for(unsigned int i = 0; i < seed_count + 4; i++) //+4 for the first inital dword (extraoptions)
        CT_NextRandomRange(256);
    cd->decrypt_seed[2] = CT_a;

    CT_Decrypt(&decr, &rand, 1);
    unsigned char* signature_size = CT_Decrypt(&decr, &rand, 1);
    while(*signature_size)
    {
        real_size += (*signature_size) + 4 + 1 + 1; //chk+lvl+pubsize
        CT_Decrypt(&decr, &rand, (*signature_size) + 4);
        CT_Decrypt(&decr, &rand, 1);
        signature_size = CT_Decrypt(&decr, &rand, 1);
    }
    if(real_size)
    {
        cd->raw_data = decr_cert;
        cd->raw_size = real_size;
    }
    else
    {
        cd->raw_data = 0;
        cd->raw_size = 0;
    }
    free2(rand);
}
