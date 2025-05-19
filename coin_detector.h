//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLIT�CNICO DO C�VADO E DO AVE
//                          2024/2025
//             ENGENHARIA DE SISTEMAS INFORM�TICOS
//           VIS�O POR COMPUTADOR - TRABALHO PR�TICO
//
//                       [ GRUPO 26 ]
//					FICHEIRO - COIN_DETECTOR.H
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef COIN_DETECTOR_H
#define COIN_DETECTOR_H

#include <opencv2/opencv.hpp>

// Fun��o principal para detec��o de moedas
void detect_coins_in_frame(cv::Mat& frame, int currentFrame);

#endif
