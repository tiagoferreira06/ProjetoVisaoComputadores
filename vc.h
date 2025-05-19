//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2022/2023
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//                    VISÃO POR COMPUTADOR
//
//             [  DUARTE DUQUE - dduque@ipca.pt  ]
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


#define VC_DEBUG
#define MAX3(a,b,c) ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))
#define MIN3(a,b,c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char *data;
	int width, height;
	int channels;			// Binário/Cinzentos=1; RGB=3
	int levels;				// Binário=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

int vc_gray_negative(IVC* srcdst);

int vc_rgb_to_hsv(IVC* src, IVC* dst);

int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);

int vc_scale_gray_to_color_pallette(IVC* src, IVC* dst);

int vc_pixel_counter(IVC* src);

int vc_gray_to_binary(IVC* src, IVC* dst, int treshold);
int vc_gray_to_binary_global_mean(IVC* src, IVC* dst);
int vc_gray_to_binary_midpoint(IVC* src, IVC* dst, int kernel);

int vc_pixel_counter(IVC* src);
int vc_gray_to_binary_global_mean(IVC* src, IVC* dst);

int vc_binary_dilate(IVC* src, IVC* dst, int kernel);
int vc_binary_erode(IVC* src, IVC* dst, int kernel);

int vc_binary_open(IVC* src, IVC* dst, int kernel);
int vc_binary_close(IVC* src, IVC* dst, int kernel);

int vc_gray_to_binary2(IVC* src, IVC* dst, int treshold1, int treshold2);
int vc_paint_brain(IVC* src, IVC* bin, IVC* dst);

int vc_binary_blob_labelling(IVC* src, IVC* dst);



//Estrutura Blob Etiqueta

typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// Área
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Perímetro
	int label;					// Etiqueta
} OVC;

#define MAX(a,b) ((a) > (b) ? (a) : (b))

//Funçoes Blob

OVC* vc_binary_blob_labelling2(IVC* src, IVC* dst, int* nlabels);
int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs);


int vc_join_segmentations(IVC* src1, IVC* src2, IVC* dst);


