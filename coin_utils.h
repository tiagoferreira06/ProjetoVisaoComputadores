//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2024/2025
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//           VIS�O POR COMPUTADOR - TRABALHO PR�TICO
//
//                       [ GRUPO 26 ]
//					FICHEIRO - COIN_UTILS.H
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef COIN_UTILS_H
#define COIN_UTILS_H

#include <opencv2/opencv.hpp>
#include <string>

// Estrutura para rastrear moedas
struct CoinTrack {
    cv::Rect bbox;            // Bounding box da moeda
    std::string type;         // Tipo da moeda (1 c�ntimo, 2 c�ntimos, ...)

    int lastSeenFrame;        // �ltimo frame em que a moeda foi vista
    int firstSeenFrame;       // Primeiro frame em que a moeda foi vista
    bool counted;             // Flag para indicar se a moeda j� foi contada
    bool typeConfirmed;       // Flag para indicar se o tipo est� confirmado
    int finalArea;            // �rea final confirmada para a moeda
    int finalPerimeter;       // Per�metro final confirmado para a moeda
    float finalCircularity;   // Circularidade final confirmada para a moeda

    cv::Point fixedCenter;
    std::vector<int> areaHistory;      // Hist�rico das �ltimas �reas detectadas
    std::vector<int> perimeterHistory; // Hist�rico dos �ltimos per�metros detectados
    bool matched_this_frame;           // Flag para indicar se foi correspondida neste frame

    // Construtor para inicializar
    CoinTrack() : counted(false), typeConfirmed(false), finalArea(0), finalPerimeter(0),
        finalCircularity(0.0f), matched_this_frame(false) {
    }
};

// Fun��es auxiliares
float calculate_radius(int area);
float calculate_circularity(int area, int perimeter);
const char* classify_coin(int area, float circularity, const std::string& color);
double calculateDistance(const cv::Rect& r1, const cv::Rect& r2);
int findMatchingCoin(const cv::Rect& newBBox, int currentFrame);
void drawCenter(cv::Mat& frame, cv::Point center, int radius, cv::Vec3b color);
void drawRectangleManual(cv::Mat& frame, cv::Rect rect, cv::Vec3b color);
void vc_timer(void);

// Constantes globais
extern const int FRAME_THRESHOLD;
extern const int MAX_DISTANCE;
extern const int STABILITY_THRESHOLD;
extern const float MAX_AREA_VARIATION;
extern const float MIN_CIRCULARITY;

// Vari�veis globais
extern std::vector<CoinTrack> trackedCoins;
extern std::map<std::string, int> coinCount;

#endif
