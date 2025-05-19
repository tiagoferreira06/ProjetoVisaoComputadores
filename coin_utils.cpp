//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2024/2025
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//           VISÃO POR COMPUTADOR - TRABALHO PRÁTICO
//
//                       [ GRUPO 26 ]
//					FICHEIRO - COIN_UTILS.CPP
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define _CRT_SECURE_NO_WARNINGS

#include "coin_utils.h"
#include <iostream>
#include <cmath>
#include <chrono>

// Definição das constantes globais
const int FRAME_THRESHOLD = 30;
const int MAX_DISTANCE = 60;
const int STABILITY_THRESHOLD = 5;
const float MAX_AREA_VARIATION = 0.05f;
const float MIN_CIRCULARITY = 0.11f;

// Definição das variáveis globais
std::vector<CoinTrack> trackedCoins;
std::map<std::string, int> coinCount;

// Função para calcular o raio do circulo(moeda) baseado na area
float calculate_radius(int area) {
    return sqrt(area / 3.14);
}

// Função para calcular a circularidade da moeda baseada na área e perímetro
float calculate_circularity(int area, int perimeter) {
    if (perimeter == 0) return 0;
    return (4.0f * 3.14 * area) / (perimeter * perimeter);
}

// Função para classificar moedas baseado na área, circularidade e cor
const char* classify_coin(int area, float circularity, const std::string& color) {
    // Moedas pequenas, de cobre
    if (area >= 8000 && area < 11000 && color == "cooper") return "1 centimo";
    else if (area >= 13000 && area < 15000 && circularity > 0.9) return "2 centimos";
    else if (area >= 17000 && area < 18500 && circularity > 0.7) return "5 centimos";

    // Moedas douradas
    else if (area > 10000 && area <= 16800 && color == "gold") return "10 centimos";
    else if (area >= 19000 && area < 22000 && color == "gold") {
        return "20 centimos";
    }
    // Moedas com combinação de cor e forma
    else if (area >= 10000 && area < 22000 && (color == "silver" || circularity < 0.5)) {
        return "1 euro";
    }
    else if (area >= 22300 && area <= 25000) {
        return "50 centimos";
    }
    else if (area >= 25000 && color == "gold") {
        return "2 euros";
    }

    return "A Carregar...";
}

// Função para calcular a distancia entre os centros de dois retanguloss
double calculateDistance(const cv::Rect& r1, const cv::Rect& r2) {
    cv::Point center1(r1.x + r1.width / 2, r1.y + r1.height / 2);
    cv::Point center2(r2.x + r2.width / 2, r2.y + r2.height / 2);

    return sqrt(pow(center1.x - center2.x, 2) + pow(center1.y - center2.y, 2));
}

// Função para verificar se uma nova detecao corresponde a uma moeda já rastreada
int findMatchingCoin(const cv::Rect& newBBox, int currentFrame) {
    for (size_t i = 0; i < trackedCoins.size(); i++) {
        double distance = calculateDistance(newBBox, trackedCoins[i].bbox);

        // Se a distância for menor que o limite, consideramos a mesma moeda
        if (distance < MAX_DISTANCE) {
            return i;  // Retorna o índice da moeda rastreada
        }
    }

    return -1;  // Nenhuma correspondência encontrada
}

//Função para desenhar centro de massa da moeda
void drawCenter(cv::Mat& frame, cv::Point center, int radius, cv::Vec3b color) {
    int rows = frame.rows;
    int cols = frame.cols;

    int x_start = std::max(center.x - radius, 0);
    int x_end = std::min(center.x + radius, cols - 1);
    int y_start = std::max(center.y - radius, 0);
    int y_end = std::min(center.y + radius, rows - 1);

    for (int y = y_start; y <= y_end; y++) {
        for (int x = x_start; x <= x_end; x++) {
            frame.at<cv::Vec3b>(y, x) = color;
        }
    }
}

//Função para desenhar bouding box da moeda
void drawRectangleManual(cv::Mat& frame, cv::Rect rect, cv::Vec3b color) {
    int rows = frame.rows;
    int cols = frame.cols;
    int thickness = 3;

    for (int t = 0; t < thickness; t++) {
        // Linha superior horizontal
        int y = rect.y + t;
        if (y >= 0 && y < rows) {
            for (int x = rect.x; x < rect.x + rect.width; x++) {
                if (x >= 0 && x < cols)
                    frame.at<cv::Vec3b>(y, x) = color;
            }
        }   

        // Linha inferior horizontal
        y = rect.y + rect.height - 1 - t;
        if (y >= 0 && y < rows) {
            for (int x = rect.x; x < rect.x + rect.width; x++) {
                if (x >= 0 && x < cols)
                    frame.at<cv::Vec3b>(y, x) = color;
            }
        }

        // Linha vertical esquerda
        int x = rect.x + t;
        if (x >= 0 && x < cols) {
            for (int y = rect.y; y < rect.y + rect.height; y++) {
                if (y >= 0 && y < rows)
                    frame.at<cv::Vec3b>(y, x) = color;
            }
        }

        // Linha vertical direita
        x = rect.x + rect.width - 1 - t;
        if (x >= 0 && x < cols) {
            for (int y = rect.y; y < rect.y + rect.height; y++) {
                if (y >= 0 && y < rows)
                    frame.at<cv::Vec3b>(y, x) = color;
            }
        }
    }
}


// Função para medir o tempo de execução
void vc_timer(void) {
    static bool running = false;
    static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

    if (!running) {
        running = true;
    }
    else {
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

        // Tempo em segundos
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
        double nseconds = time_span.count();

        std::cout << "Tempo decorrido: " << nseconds << " segundos" << std::endl;
        std::cout << "Pressione qualquer tecla para continuar...\n";
        std::cin.get();
    }
}