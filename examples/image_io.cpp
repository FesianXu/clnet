/*
 * image_io.cpp
 *
 *  Created on: 2017��8��22��
 *      Author: ZhangHua
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <algorithm>

#include <image_io.hpp>
#include <device_instance.hpp>

using namespace std;
using namespace clnet;

int reverse_int(int i) {
	unsigned char ch1, ch2, ch3, ch4;
	ch1 = i & 255;
	ch2 = (i >> 8) & 255;
	ch3 = (i >> 16) & 255;
	ch4 = (i >> 24) & 255;
	return((int)ch1 << 24) + ((int)ch2 << 16) + ((int)ch3 << 8) + ch4;
}

Tensor* read_mnist_images(string file, string name, int alignment_size) {
	ifstream ifs(file, ios::binary);
	if (!ifs)
		throw runtime_error("failed to open " + file);

	int magic_number, n_cols, n_rows, num_of_images;
	ifs.read((char*) &magic_number, sizeof(magic_number));
	magic_number = reverse_int(magic_number);
	ifs.read((char*) &num_of_images, sizeof(num_of_images));
	num_of_images = reverse_int(num_of_images);
	ifs.read((char*) &n_rows, sizeof(n_rows));
	n_rows = reverse_int(n_rows);
	ifs.read((char*) &n_cols, sizeof(n_cols));
	n_cols = reverse_int(n_cols);

	int num = (num_of_images + alignment_size - 1) / alignment_size * alignment_size;
	auto tensor = new Tensor({num, n_cols, n_rows}, {}, name);
	tensor->initialize(nullptr);
	float* p = tensor->pointer;
	unsigned char temp;
	for (int i = 0; i < num_of_images; ++i) {
		for (int r = 0; r < n_rows; ++r) {
			for (int c = 0; c < n_cols; ++c) {
				ifs.read((char*) &temp, sizeof(temp));
				*p++ = (float) temp;
			}
		}
	}
	ifs.close();
	memcpy(p, tensor->pointer, (num - num_of_images) * n_cols * n_rows * sizeof(float));
	return tensor;
}

Tensor* read_mnist_labels(string file, string name, int alignment_size) {
	ifstream ifs(file, ios::binary);
	if (!ifs)
		throw runtime_error("failed to open " + file);

	int magic_number, num_of_images;
	ifs.read((char*) &magic_number, sizeof(magic_number));
	magic_number = reverse_int(magic_number);
	ifs.read((char*) &num_of_images, sizeof(num_of_images));
	num_of_images = reverse_int(num_of_images);

	int num = (num_of_images + alignment_size - 1) / alignment_size * alignment_size;
	auto tensor = new Tensor({num}, {}, name);
	tensor->initialize(nullptr);
	float* p = tensor->pointer;
	unsigned char temp;
	for (int i = 0; i < num_of_images; ++i) {
		ifs.read((char*) &temp, sizeof(temp));
		*p++ = (float) temp;
	}
	ifs.close();
	memcpy(p, tensor->pointer, (num - num_of_images) * sizeof(float));
	return tensor;
}

typedef long LONG;
typedef unsigned long DWORD;
typedef unsigned short WORD;

// λͼ�ļ�ͷ�ļ�����
#pragma pack(push, 2)
typedef struct {
	WORD    bfType; //�ļ����ͣ�������0x424D,���ַ���BM��
	DWORD   bfSize; //�ļ���С
	WORD    bfReserved1; //������
	WORD    bfReserved2; //������
	DWORD   bfOffBits; //���ļ�ͷ��ʵ��λͼ���ݵ�ƫ���ֽ���
} BMPFILEHEADER;
#pragma pack(pop)

typedef struct{
	DWORD      biSize; //��Ϣͷ��С
	LONG       biWidth; //ͼ����
	LONG       biHeight; //ͼ��߶�
	WORD       biPlanes; //λƽ����������Ϊ1
	WORD       biBitCount; //ÿ����λ��
	DWORD      biCompression; //ѹ������
	DWORD      biSizeImage; //ѹ��ͼ���С�ֽ���
	LONG       biXPelsPerMeter; //ˮƽ�ֱ���
	LONG       biYPelsPerMeter; //��ֱ�ֱ���
	DWORD      biClrUsed; //λͼʵ���õ���ɫ����
	DWORD      biClrImportant; //��λͼ����Ҫ��ɫ����
} BMPINFOHEADER; //λͼ��Ϣͷ����

void generate_24bits_bmp(unsigned char* pData, int width, int height, const char* file) //����BmpͼƬ������RGBֵ������ͼƬ���ش�С������ͼƬ�洢·��
{
	int size = width * height * 3; //�������ݴ�С
	// λͼ��һ���֣��ļ���Ϣ
	BMPFILEHEADER bfh;
	bfh.bfType = 0x4D42; //bm
	bfh.bfSize = size + sizeof(BMPFILEHEADER) + sizeof(BMPINFOHEADER);
	bfh.bfReserved1 = 0; //reserved
	bfh.bfReserved2 = 0; //reserved
	bfh.bfOffBits = sizeof(BMPFILEHEADER) + sizeof(BMPINFOHEADER);

	// λͼ�ڶ����֣�������Ϣ
	BMPINFOHEADER bih;
	bih.biSize = sizeof(BMPINFOHEADER);
	bih.biWidth = width;
	bih.biHeight = height;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = 0;
	bih.biSizeImage = size;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	FILE* fp = fopen(file,"wb");
	if (!fp)
		return;
	fwrite(&bfh, sizeof(BMPFILEHEADER), 1, fp);
	fwrite(&bih, sizeof(BMPINFOHEADER), 1, fp);
	fwrite(pData, 1, size, fp);
	fclose(fp);
}
