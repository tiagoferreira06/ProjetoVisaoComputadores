//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2024/2025
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//           VISÃO POR COMPUTADOR - TRABALHO PRÁTICO
//
//                       [ GRUPO 26 ]
//					FICHEIRO - COIN_DETECTOR.H
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef COIN_DETECTOR_H
#define COIN_DETECTOR_H

#include <opencv2/opencv.hpp>

// Função principal para detecção de moedas
void detect_coins_in_frame(cv::Mat& frame, int currentFrame);

#endif
