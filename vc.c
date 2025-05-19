//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Desabilita (no MSVC++) warnings de funções não seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar memória para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *) malloc(sizeof(IVC));

	if(image == NULL) return NULL;
	if((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

	if(image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar memória de uma imagem
IVC *vc_image_free(IVC *image)
{
	if(image != NULL)
	{
		if(image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;
	
	for(;;)
	{
		while(isspace(c = getc(file)));
		if(c != '#') break;
		do c = getc(file);
		while((c != '\n') && (c != EOF));
		if(c == EOF) break;
	}
	
	t = tok;
	
	if(c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));
		
		if(c == '#') ungetc(c, file);
	}
	
	*t = 0;
	
	return tok;
}


long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);
				
				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;
				
				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}


IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;
	
	// Abre o ficheiro
	if((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if(strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if(strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if(strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
			#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
			#endif

			fclose(file);
			return NULL;
		}
		
		if(levels == 1) // PBM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			if((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			size = image->width * image->height * image->channels;

			if((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}
		
		fclose(file);
	}
	else
	{
		#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
		#endif
	}
	
	return image;
}


int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;
	
	if(image == NULL) return 0;

	if((file = fopen(filename, "wb")) != NULL)
	{
		if(image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;
			
			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);
			
			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if(fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);
		
			if(fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				return 0;
			}
		}
		
		fclose(file);

		return 1;
	}
	
	return 0;
}

// Gerar negativo da imagem Grey
int vc_gray_negative(IVC* srcdst)
{
	unsigned char* data = srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Inverte a imagem Gray
	for (y = 0;y < height;y++) 
	{
		for (x = 0;x < width;x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = 255 - data[pos];
		}
	}
	return 1;
}

// Gerar negativo da imagem RGB
int vc_rgb_negative(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	// Verificação de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	// Inverte a imagem RGB
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;

			data[pos] = 255 - data[pos];
			data[pos + 1] = 255 - data[pos + 1];
			data[pos + 2] = 255 - data[pos + 2];
		}
	}

	return 1;
}

// Converter de RGB para Gray
int vc_rgb_to_gray(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}
	}

	return 1;
}

int vc_rgb_to_hsv(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_src = src->width * src->channels;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_src = src->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf, max, min, hue, sat;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 3)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			max = MAX3(rf, gf, bf);
			min = MIN3(rf, gf, bf);

			if ((max == min) || (max == 0))
			{
				hue = 0;
				sat = 0;

			}
			else {

				sat = (max - min) / max * 255.0f;

				if (max == rf)
				{
					if (gf >= bf)
					{
						hue = 60.0f * ((gf - bf) / (max - min));
					}
					else
					{
						hue = 360.0f + 60.0f * ((gf - bf) / (max - min));
					}
				}
				else if (max == gf)
				{
					hue = 120.0f + 60.0f * ((bf - rf) / (max - min));
				}
				else if (max == bf)
				{
					hue = 240.0f + 60.0f * ((rf - gf) / (max - min));
				}
			}
			datadst[pos_dst] = (unsigned char)((hue / 360.0f) * 255.0f);
			datadst[pos_dst + 1] = (unsigned char)sat;
			datadst[pos_dst + 2] = (unsigned char)max;
		}
	}
	return 1;
}

int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char* dataSrc = (unsigned char*)src->data;
	int bytesPerLineSrc = src->width * src->channels;
	int channelsSrc = src->channels;
	unsigned char* dataDst = (unsigned char*)dst->data;
	int bytesPerLineDst = dst->width * dst->channels;
	int channelsDst = dst->channels;
	int width = src->width;
	int height = src->height;
	long int posSrc, posDst;
	float h, s, v;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;
	if (dst->levels != 255) return 0;
	if (!(hmin >= 0 && hmax <= 360)) return 0;
	if (!(smin >= 0 && smax <= 255)) return 0;
	if (!(vmin >= 0 && vmax <= 255)) return 0;

	hmin = ((float)hmin * 255) / 360;
	hmax = ((float)hmax * 255) / 360;
	smin = ((float)smin * 255) / 100;
	smax = ((float)smax * 255) / 100;
	vmin = ((float)vmin * 255) / 100;
	vmax = ((float)vmax * 255) / 100;

	for (int y = 0; y < height;  y++)
	{
		for (int x = 0; x < width; x++) 
		{
			posSrc = y * bytesPerLineSrc + x * channelsSrc;
			posDst = y * bytesPerLineDst + x * channelsDst;

			h = (float)dataSrc[posSrc];
			s = (float)dataSrc[posSrc + 1];
			v = (float)dataSrc[posSrc + 2];

			if (h >= hmin && h <= hmax && s >= smin && s <= smax && v >= vmin && v <= vmax)
			{
				dataDst[posDst] = 255;
			}
			else
			{
				dataDst[posDst] = 0;
			}
		}
	}
	return 1;
}

int vc_scale_gray_to_color_pallette(IVC* src, IVC* dst)
{
	unsigned char* dataSrc = (unsigned char*)src->data;
	int bytesPerLineSrc = src->width * src->channels;
	int channelsSrc = src->channels;
	unsigned char* dataDst = (unsigned char*)dst->data;
	int bytesPerLineDst = dst->width * dst->channels;
	int channelsDst = dst->channels;
	int width = src->width;
	int height = src->height;
	long int posSrc, posDst;
	float pgm, r, g, b;

	if ((src->width <= 0) || (src->height <= 0))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 3))return 0;
	if ((dst->levels != 255))return 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			posSrc = y * bytesPerLineSrc + x * channelsSrc;
			posDst = y * bytesPerLineDst + x * channelsDst;

			pgm = (float)dataSrc[posSrc];

			if (pgm >= 0 && pgm < 64)
			{
				r = 0;
				g = pgm * 4;
				b = 255;
			}
			else if (pgm >= 64 && pgm < 128)
			{
				r = 0;
				g = 255;
				b = (255 - (pgm - 64)) * 4;
			}
			else if (pgm >= 128 && pgm < 192)
			{
				r = (pgm - 128) * 4;
				g = 255;
				b = 0;
			}
			else if (pgm >= 192 && pgm < 255)
			{
				r = 255;
				g = (255 - (pgm - 192)) * 4;
				b = 0;
			}

			dataDst[posDst] = r;
			dataDst[posDst + 1] = g;
			dataDst[posDst + 2] = b;
		}
	}
	return 1;
}

int vc_pixel_counter(IVC* src)
{
	unsigned char* dataSrc = (unsigned char*)src->data;
	int bytesPerLineSrc = src->width * src->channels;
	int channelsSrc = src->channels;
	int width = src->width;
	int height = src->height;
	long int posSrc;
	float temp;
	int count = 0;
	float percentage = 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			posSrc = y * bytesPerLineSrc + x * channelsSrc;

			temp = (float)dataSrc[posSrc];

			if (temp == 255)
			{
				count++;
			}
		}
	}
	percentage = (count * 100) / (width * height);
	return percentage;
}

int vc_gray_to_binary(IVC* src, IVC* dst, int treshold)
{
	unsigned char* dataSrc = src->data;
	int bytesPerLineSrc = src->width * src->channels;
	int channelsSrc = src->channels;
	unsigned char* dataDst = dst->data;
	int bytesPerLineDst = dst->width * dst->channels;
	int channelsDst = dst->channels;
	int width = src->width;
	int height = src->height;
	long int posSrc, posDst;
	float temp;

	if (src->width <= 0 || src->height <= 0)return 0;
	if (src->width != dst->width || src->height != dst->height)return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			posSrc = y * bytesPerLineSrc + x * channelsSrc;
			posDst = y * bytesPerLineDst + x * channelsDst;

			temp = dataSrc[posSrc];

			if (temp > treshold)
			{
				dataDst[posDst] = 255;
			}
			if (temp <= treshold)
			{
				dataDst[posDst] = 0;
			}
		}
	}
	return 1;
}

int vc_gray_to_binary_global_mean(IVC* src, IVC* dst)
{
	unsigned char* dataSrc = src->data;
	int bytesPerLineSrc = src->width * src->channels;
	int channelsSrc = src->channels;
	unsigned char* dataDst = dst->data;
	int bytesPerLineDst = dst->width * dst->channels;
	int channelsDst = dst->channels;
	int width = src->width;
	int height = src->height;
	long int posSrc, posDst;
	int treshold;
	int count = 0;
	float temp = 0;

	if ((src->width <= 0) || (src->height <= 0))return 0;
	if ((src->width != dst->width) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			posSrc = y * bytesPerLineSrc + x * channelsSrc;

			count++;

			temp += dataSrc[posSrc];
		}
	}

	treshold = temp / count;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			posSrc = y * bytesPerLineSrc + x * channelsSrc;
			posDst = y * bytesPerLineDst + x * channelsDst;

			temp = dataSrc[posSrc];

			if (temp > treshold)
			{
				dataDst[posDst] = 255;
			}
			if (temp <= treshold)
			{
				dataDst[posDst] = 0;
			}
		}
	}
	return 1;
}

vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel)
{
	unsigned char* dataSrc = src->data;
	int bytesPerLineSrc = src->width * src->channels;
	int channelsSrc = src->channels;
	unsigned char* dataDst = dst->data;
	int bytesPerLineDst = dst->width * dst->channels;
	int channelsDst = dst->channels;
	int width = src->width;
	int height = src->height;
	long int posSrc, posDst;
	int temp = 0, count = 0;
	int offset, treshold = 0;

	if ((src->width <= 0) || (src->height <= 0))return 0;
	if ((src->width != dst->height) || (src->height != dst->height))return 0;
	if ((src->channels != 1) || (dst->channels != 1))return 0;

	offset = (kernel - 1) / 2;
	
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			posSrc = y * bytesPerLineSrc + x * channelsSrc;
			posDst = y * bytesPerLineDst + x * channelsDst;

			int vmin = 255;
			int vmax = 0;

			for (int ky = (y - offset); ky <= (y + offset); ky++)
			{
				for (int kx = (x - offset); kx <= (x + offset); kx++)
				{
					if (kx >= 0 && kx < width && ky >= 0 && ky < height)
					{
						posSrc = ky * bytesPerLineSrc + kx * channelsSrc;
						posDst = ky * bytesPerLineDst + kx * channelsDst;

						temp = dataSrc[posSrc];

						if (temp < vmin)vmin = temp;
						if (temp > vmax)vmax = temp;
					}
				}
			}
			treshold = (vmin + vmax) / 2;
			dataDst[posDst] = (dataSrc[posSrc] > treshold ? 255 : 0);
		}
	}
	return 1;
}

int vc_binary_dilate(IVC* src, IVC* dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y, nx, ny, flag;
	long int pos_src, pos_src_for, pos_dst;
	float threshold = 0;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;
	int offset = (int)(kernel) / 2;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline + x * channels_src;
			pos_dst = y * bytesperline + x * channels_dst;
			flag = 0;
			for (ny = y - offset; ny <= y + offset; ny++)
			{
				for (nx = x - offset; nx <= x + offset; nx++)
				{

					if (nx >= 0 && nx < width && ny >= 0 && ny < height)
					{

						pos_src_for = (ny)*bytesperline + (nx)*channels_src;
						if (datasrc[pos_src_for] == 255)
						{
							flag = 1;
						}

					}
				}
			}
			if (flag == 1) {
				datadst[pos_dst] = 255;
			}
			else {
				datadst[pos_dst] = 0;
			}
		}
	}
}

int vc_binary_erode(IVC* src, IVC* dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y, nx, ny, flag;
	long int pos_src, pos_src_for, pos_dst;
	float threshold = 0;

	//Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;
	int offset = (int)(kernel) / 2;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline + x * channels_src;
			pos_dst = y * bytesperline + x * channels_dst;
			flag = 0;
			for (ny = y - offset; ny <= y + offset; ny++)
			{
				for (nx = x - offset; nx <= x + offset; nx++)
				{

					if (nx >= 0 && nx < width && ny >= 0 && ny < height)
					{

						pos_src_for = (ny)*bytesperline + (nx)*channels_src;
						if (datasrc[pos_src_for] == 0)
						{
							flag = 1;
						}

					}
				}
			}
			if (flag == 1) {
				datadst[pos_dst] = 0;
			}
			else {
				datadst[pos_dst] = 255;
			}
		}
	}
}

int vc_binary_open(IVC* src, IVC* dst, int kernel) 
{
	IVC* temp;

	temp = vc_image_new(src->width, src->height, 1, 255);
	if (temp == NULL)
	{
		printf("ERROR -> vc_read_image():\n\Out of memory!\n");
		(void)getchar();
		return 0;
	}

	vc_binary_erode(src, temp, kernel);
	vc_binary_dilate(temp, dst, kernel);

	free(temp);
}

int vc_binary_close(IVC* src, IVC* dst, int kernel)
{
	IVC* temp;

	temp = vc_image_new(src->width, src->height, 1, 255);
	if (temp == NULL)
	{
		printf("ERROR -> vc_read_image():\n\Out of memory!\n");
		(void)getchar();
		return 0;
	}
	
	vc_binary_dilate(src, temp, kernel);
	vc_binary_erode(temp, dst, kernel);

	free(temp);
}

int vc_gray_to_binary2(IVC* src, IVC* dst, int treshold1, int treshold2)
{
	unsigned char* dataSrc = src->data;
	int bytesPerLineSrc = src->width * src->channels;
	int channelsSrc = src->channels;
	unsigned char* dataDst = dst->data;
	int bytesPerLineDst = dst->width * dst->channels;
	int channelsDst = dst->channels;
	int width = src->width;
	int height = src->height;
	long int posSrc, posDst;
	float temp;

	if (src->width <= 0 || src->height <= 0)return 0;
	if (src->width != dst->width || src->height != dst->height)return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			posSrc = y * bytesPerLineSrc + x * channelsSrc;
			posDst = y * bytesPerLineDst + x * channelsDst;

			temp = dataSrc[posSrc];

			if (temp > treshold1 && temp < treshold2)
			{
				dataDst[posDst] = 255;
			}
			else
			{
				dataDst[posDst] = 0;
			}
		}
	}
	return 1;
}

int vc_paint_brain(IVC* src, IVC* bin, IVC* dst)
{
	unsigned char* dataSrc = src->data;
	unsigned char* dataDst = dst->data;
	unsigned char* dataBin = bin->data;
	int bytesPerLineSrc = src->width * src->channels;
	int bytesPerLineDst = dst->width * dst->channels;
	int bytesPerLineBin = bin->width * bin->channels;
	int channelsSrc = src->channels;
	int channelsDst = dst->channels;
	int channelsBin = bin->channels;
	int width = src->width;
	int height = src->height;
	long int posSrc, posDst, posBin;
	

	if (src->width <= 0 || src->height <= 0)return 0;
	if (src->width != dst->width || src->height != dst->height)return 0;
	if (src->width != bin->width || src->height != bin->height)return 0;
	if (src->channels != 1 || dst->channels != 1 || bin->channels != 1)return 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			posSrc = y * bytesPerLineSrc + x * channelsSrc;
			posDst = y * bytesPerLineDst + x * channelsDst;
			posBin = y * bytesPerLineBin + x * channelsBin;

			if (dataSrc[posSrc] == 255)
			{
				dataDst[posDst] = dataBin[posBin];
			}
			else
			{
				dataDst[posDst] = 0;
			}
		}
	}
}

int vc_binary_blob_labelling(IVC* src, IVC* dst)
{
	unsigned char* dataSrc = src->data;
	unsigned char* dataDst = dst->data;
	int bytesPerLineSrc = src->width * src->channels;
	int bytesPerLineDst = dst->width * dst->channels;
	int channelsSrc = src->channels;
	int channelsDst = dst->channels;
	int width = src->width;
	int height = src->height;
	int label = 1;
	int labelMin;
	long int posSrc, posDst, posA, posB, posC, posD;


	if (src->width <= 0 || src->height <= 0)return 0;
	if (src->width != dst->width || src->height != dst->height)return 0;
	if (src->channels != 1 || dst->channels != 1)return 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			posSrc = y * bytesPerLineSrc + x * channelsSrc;
			posDst = y * bytesPerLineDst + x * channelsDst;
			posA = (y - 1) * bytesPerLineSrc + (x - 1);
			posB = posA + 1;
			posC = posA + 2;
			posD = posSrc - 1;
			
			labelMin = label;

			dataDst[posDst] = dataSrc[posSrc];

			if (dataDst[posDst] == 255)
			{
				if (dataDst[posA] == 0 && dataDst[posB] == 0 && dataDst[posC] == 0 && dataDst[posD] == 0)
				{
					dataDst[posDst] = label;
					label++;
				}
				else
				{
					if (dataDst[posA] != 0 && dataDst[posA] < labelMin) labelMin = dataDst[posA];
					if (dataDst[posB] != 0 && dataDst[posB] < labelMin) labelMin = dataDst[posB];
					if (dataDst[posC] != 0 && dataDst[posC] < labelMin) labelMin = dataDst[posC];
					if (dataDst[posC] != 0 && dataDst[posC] < labelMin) labelMin = dataDst[posD];

					dataDst[posDst] = labelMin;
				}
			}
			else dataDst[posDst] = 0;
		}
	}
	return 1;
}

// Etiquetagem de blobs
// src		: Imagem de entrada
// dst		: Imagem que irá conter as etiquetas
// nlabels	: Endereço de memória de uma variável, onde será armazenado o número de labels encontradas.
// OVC*		: Retorna um array de estruturas de blobs (objectos), com respectivas etiquetas.
// Versão melhorada, esta suporta mais do que 255 labels
OVC* vc_binary_blob_labelling2(IVC* src, IVC* dst, int* nlabels)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned int* datadst_int; 
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int* labeltable;
	int* labelarea;
	int* labelperimeter; 
	int label = 1; 
	int num, tmplabel;
	OVC* blobs; 

	
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	
	datadst_int = (unsigned int*)malloc(width * height * sizeof(unsigned int));
	if (datadst_int == NULL) return NULL;

	memset(datadst_int, 0, width * height * sizeof(unsigned int));

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			if (datasrc[y * bytesperline + x * channels] != 0) {
				datadst_int[y * width + x] = 65535; 
			}
		}
	}
	
	labeltable = (int*)malloc((width * height + 1) * sizeof(int));
	labelarea = (int*)malloc((width * height + 1) * sizeof(int));
	labelperimeter = (int*)malloc((width * height + 1) * sizeof(int)); 
	if (labeltable == NULL || labelarea == NULL || labelperimeter == NULL) {
		free(datadst_int);
		if (labeltable) free(labeltable);
		if (labelarea) free(labelarea);
		if (labelperimeter) free(labelperimeter);
		return NULL;
	}
	
	for (i = 0; i <= width * height; i++) {
		labeltable[i] = i;
		labelarea[i] = 0;
		labelperimeter[i] = 0; 
	}

	for (y = 0; y < height; y++) {
		datadst_int[y * width + 0] = 0;
		datadst_int[y * width + (width - 1)] = 0;
	}
	for (x = 0; x < width; x++) {
		datadst_int[0 * width + x] = 0;
		datadst_int[(height - 1) * width + x] = 0;
	}

	for (y = 1; y < height - 1; y++) {
		for (x = 1; x < width - 1; x++) {

			posA = (y - 1) * width + (x - 1); 
			posB = (y - 1) * width + x;       
			posC = (y - 1) * width + (x + 1); 
			posD = y * width + (x - 1);       
			posX = y * width + x;             

			if (datadst_int[posX] != 0) {
				if ((datadst_int[posA] == 0) && (datadst_int[posB] == 0) &&
					(datadst_int[posC] == 0) && (datadst_int[posD] == 0)) {
					datadst_int[posX] = label;
					labeltable[label] = label;
					label++;

					if (label >= width * height) {
						free(datadst_int);
						free(labeltable);
						free(labelarea);
						free(labelperimeter);
						return NULL;
					}
				}
				else {
					num = 65535; 

					// Se A está marcado
					if (datadst_int[posA] != 0) num = labeltable[datadst_int[posA]];
					// Se B está marcado, e é menor que a etiqueta "num"
					if ((datadst_int[posB] != 0) && (labeltable[datadst_int[posB]] < num))
						num = labeltable[datadst_int[posB]];
					// Se C está marcado, e é menor que a etiqueta "num"
					if ((datadst_int[posC] != 0) && (labeltable[datadst_int[posC]] < num))
						num = labeltable[datadst_int[posC]];
					// Se D está marcado, e é menor que a etiqueta "num"
					if ((datadst_int[posD] != 0) && (labeltable[datadst_int[posD]] < num))
						num = labeltable[datadst_int[posD]];

					datadst_int[posX] = num;
					labeltable[num] = num;

					if (datadst_int[posA] != 0) {
						if (labeltable[datadst_int[posA]] != num) {
							for (tmplabel = labeltable[datadst_int[posA]], a = 1; a < label; a++) {
								if (labeltable[a] == tmplabel) {
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst_int[posB] != 0) {
						if (labeltable[datadst_int[posB]] != num) {
							for (tmplabel = labeltable[datadst_int[posB]], a = 1; a < label; a++) {
								if (labeltable[a] == tmplabel) {
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst_int[posC] != 0) {
						if (labeltable[datadst_int[posC]] != num) {
							for (tmplabel = labeltable[datadst_int[posC]], a = 1; a < label; a++) {
								if (labeltable[a] == tmplabel) {
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst_int[posD] != 0) {
						if (labeltable[datadst_int[posD]] != num) {
							for (tmplabel = labeltable[datadst_int[posD]], a = 1; a < label; a++) {
								if (labeltable[a] == tmplabel) {
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	for (y = 1; y < height - 1; y++) {
		for (x = 1; x < width - 1; x++) {
			posX = y * width + x; // X

			if (datadst_int[posX] != 0) {
				datadst_int[posX] = labeltable[datadst_int[posX]];
				labelarea[datadst_int[posX]]++;

				int neighbors[4] = {
					(y > 0) ? datadst_int[(y - 1) * width + x] : 0,         
					(y < height - 1) ? datadst_int[(y + 1) * width + x] : 0,    
					(x > 0) ? datadst_int[y * width + (x - 1)] : 0,           
					(x < width - 1) ? datadst_int[y * width + (x + 1)] : 0      
				};

				for (int neighbor = 0; neighbor < 4; neighbor++) {
					if (neighbors[neighbor] == 0) {
						labelperimeter[datadst_int[posX]]++;
						break;
					}
				}
			}
		}
	}

	for (a = 1; a < label - 1; a++) {
		for (b = a + 1; b < label; b++) {
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}

	*nlabels = 0;
	for (a = 1; a < label; a++) {
		if (labeltable[a] != 0) {
			labeltable[*nlabels] = labeltable[a]; 
			labelarea[*nlabels] = labelarea[labeltable[a]]; 
			labelperimeter[*nlabels] = labelperimeter[labeltable[a]]; 
			(*nlabels)++; 
		}
	}

	if (*nlabels == 0) {
		free(datadst_int);
		free(labeltable);
		free(labelarea);
		free(labelperimeter);
		return NULL;
	}

	blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL) {
		for (a = 0; a < (*nlabels); a++) {
			blobs[a].label = labeltable[a];
			blobs[a].area = labelarea[a];
			blobs[a].perimeter = labelperimeter[a]; 

			blobs[a].x = width;
			blobs[a].y = height;
			blobs[a].width = 0;
			blobs[a].height = 0;
		}
	}
	else {
		free(datadst_int);
		free(labeltable);
		free(labelarea);
		free(labelperimeter);
		return NULL;
	}

	for (y = 1; y < height - 1; y++) {
		for (x = 1; x < width - 1; x++) {
			posX = y * width + x;

			if (datadst_int[posX] != 0) {

				for (a = 0; a < *nlabels; a++) {
					if (labeltable[a] == datadst_int[posX]) {
						if (x < blobs[a].x) blobs[a].x = x;
						if (y < blobs[a].y) blobs[a].y = y;
						if (x > blobs[a].width) blobs[a].width = x;
						if (y > blobs[a].height) blobs[a].height = y;
						break;
					}
				}
			}
		}
	}

	for (a = 0; a < *nlabels; a++) {
		blobs[a].width = blobs[a].width - blobs[a].x + 1;
		blobs[a].height = blobs[a].height - blobs[a].y + 1;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			if (datadst_int[y * width + x] > 0) {
				datadst[y * bytesperline + x * channels] = 255;
			}
			else {
				datadst[y * bytesperline + x * channels] = 0;
			}
		}
	}

	free(datadst_int);
	free(labeltable);
	free(labelarea);
	free(labelperimeter);
	return blobs;
}


int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					blobs[i].area++;

					sumx += x;
					sumy += y;

					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}

int vc_gray_lowpass_mean_filter(IVC* src, IVC* dst, int kernelsize)
{
	unsigned char* dataSrc = src->data;
	unsigned char* dataDst = dst->data;
	int bytesPerLineSrc = src->width * src->channels;
	int bytesPerLineDst = dst->width * dst->channels;
	int channelsSrc = src->channels;
	int channelsDst = dst->channels;
	int width = src->width;
	int height = src->height;
	long int posSrc, posDst;


}

int vc_join_segmentations(IVC* src1, IVC* src2, IVC* dst) {
	unsigned char* data1 = (unsigned char*)src1->data;
	unsigned char* data2 = (unsigned char*)src2->data;
	unsigned char* dataout = (unsigned char*)dst->data;
	int width = src1->width;
	int height = src1->height;
	int bytesperline1 = src1->bytesperline;
	int bytesperline2 = src2->bytesperline;
	int bytesperlineOut = dst->bytesperline;
	int channels = src1->channels;
	int x, y;
	long int pos1, pos2, posOut;

	// Verificar compatibilidade das imagens
	if ((src1->width != src2->width) || (src1->width != dst->width) ||
		(src1->height != src2->height) || (src1->height != dst->height) ||
		(src1->channels != src2->channels) || (src1->channels != dst->channels) ||
		(src1->channels != 1))
	{
		printf("vc_join_segmentations(): Erro - Imagens com dimensões ou número de canais diferentes.\n");
		return 0;
	}

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos1 = y * bytesperline1 + x * channels;
			pos2 = y * bytesperline2 + x * channels;
			posOut = y * bytesperlineOut + x * channels;

			if (data1[pos1] > 0 || data2[pos2] > 0) {
				dataout[posOut] = 255;
			}
			else {
				dataout[posOut] = 0;
			}
		}
	}

	return 1;
}


