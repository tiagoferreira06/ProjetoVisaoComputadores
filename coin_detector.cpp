//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2024/2025
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//           VISÃO POR COMPUTADOR - TRABALHO PRÁTICO
//
//                       [ GRUPO 26 ]
//			       FICHEIRO - COIN_DETECTOR.CPP
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define _CRT_SECURE_NO_WARNINGS

#include "coin_detector.h"
#include "coin_utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

extern "C" {
#include "vc.h"
}

// Função para processar um frame do vídeo e detectar moedas
void detect_coins_in_frame(cv::Mat& frame, int currentFrame) {

    int width = frame.cols;
    int height = frame.rows;
    int nlabels = 0;
    static int TotalCoins = 0;

    OVC* blobs;

    // Criar imagens IVC para processamento
    IVC* image[10];

    // Converter frame cv::Mat para IVC (BGR para RGB)
    cv::Mat frame_rgb;
    cv::cvtColor(frame, frame_rgb, cv::COLOR_BGR2RGB);

    image[0] = vc_image_new(width, height, 3, 255);
    if (image[0] == NULL) {
        std::cerr << "Erro ao criar imagem principal!" << std::endl;
        return;
    }

    // Copiar dados do frame para a imagem IVC
    memcpy(image[0]->data, frame_rgb.data, width * height * 3);

    // Criar imagens auxiliares
    for (int i = 1; i < 10; i++) {
        image[i] = vc_image_new(width, height, (i == 1) ? 3 : 1, 255);
        if (image[i] == NULL) {
            std::cerr << "Erro ao criar imagens auxiliares!" << std::endl;
            for (int j = 0; j < i; j++) {
                vc_image_free(image[j]);
            }
            return;
        }
    }

    // Processamento de imagem
    vc_rgb_to_hsv(image[0], image[1]);

    // Segmentação HSV
    vc_hsv_segmentation(image[1], image[2], 20, 40, 30, 100, 10, 50); //  copper
    vc_hsv_segmentation(image[1], image[6], 43, 72, 19, 70, 7, 64); // gold
    vc_hsv_segmentation(image[1], image[8], 50, 150, 1, 25, 10, 55); // silver

    // Junção das Segmentações HSV para só uma imagem
    vc_join_segmentations(image[2], image[6], image[7]);
    vc_join_segmentations(image[7], image[8], image[9]);

    // Operações morfológicas
    vc_binary_open(image[9], image[3], 3);
    vc_binary_close(image[3], image[4], 3);

    // Etiquetagem dos blobs(moedas)
    blobs = vc_binary_blob_labelling2(image[4], image[5], &nlabels);

    if (blobs != NULL && nlabels > 0) {
        // PRIMEIRO: Marcar todas as moedas existentes como não vistas neste frame
        for (auto& coin : trackedCoins) {
            coin.matched_this_frame = false;
        }

        // SEGUNDO: Para cada blob detectado, encontrar correspondência
        for (int i = 0; i < nlabels; i++) {

			if (blobs[i].area < 7000) continue; // Ignora blobs pequenos
            float circularity = calculate_circularity(blobs[i].area, blobs[i].perimeter);
            if (circularity < MIN_CIRCULARITY) continue; // Ignora blobs com formas poucas circulares

            cv::Rect rect(blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].height);
            int matchIndex = findMatchingCoin(rect, currentFrame);

            cv::Point center(rect.x + rect.width / 2, rect.y + rect.height / 2);

            cv::Vec3b white(255, 255, 0);  
            drawCenter(frame, center, 2, white);  

            if (matchIndex >= 0) {

                cv::Vec3b pixel = frame_rgb.at<cv::Vec3b>(center);
                cv::Mat pixelRGB(1, 1, CV_8UC3, pixel);
                cv::Mat pixelHSV;
                cv::cvtColor(pixelRGB, pixelHSV, cv::COLOR_RGB2HSV);
                cv::Vec3b hsv = pixelHSV.at<cv::Vec3b>(0, 0);

                // hsv[0] = Hue, hsv[1] = Saturation, hsv[2] = Value
                int hue = hsv[0];
                int sat = hsv[1];
                int val = hsv[2];

                // Lógica para distinguir moeda com base na cor do centro
                std::string centerColor;
                if (hue >= 25 && hue <= 30 && sat >= 20 && sat <= 140 && val >= 65 && val <= 150) {
                    centerColor = "gold";
                }
                else if (hue >= 15 && hue <= 60 && sat >= 1 && sat <= 60 && val >= 10 && val <= 120) {
                    centerColor = "silver";
                }
                else if (hue >= 10 && hue <= 60 && sat >= 80 && sat <= 215 && val >= 40 && val <= 120) {
                    centerColor = "cooper";
                }
                else {
                    centerColor = "indefinido";
                }

                std::cout << "Cor do centro: " << centerColor << " (H:" << hue << " S:" << sat << " V:" << val << ")" << std::endl;

                // Marcar como correspondida
                trackedCoins[matchIndex].matched_this_frame = true;
                trackedCoins[matchIndex].bbox = rect;
                trackedCoins[matchIndex].lastSeenFrame = currentFrame;

                // Se a moeda ja esta confirmada, mostrar area e perímetro final
                if (trackedCoins[matchIndex].typeConfirmed) {
                    // VERDE - moeda confirmada
                    cv::Vec3b Green(0, 255, 0);
                    drawRectangleManual(frame, rect, Green);


                    // Mostrar TIPO, AREA, PERIMETRO e CIRCULARIDADE final da mnoeda
                    char circularity_str[10];
                    cv::putText(frame, trackedCoins[matchIndex].type,
                        cv::Point(rect.x + 25, rect.y - 40),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

                    sprintf(circularity_str, "%.2f", trackedCoins[matchIndex].finalCircularity);
                    std::string infoText =
                        "(A:" + std::to_string(trackedCoins[matchIndex].finalArea) +
                        " P:" + std::to_string(trackedCoins[matchIndex].finalPerimeter) +
                        " C:" + circularity_str + ")";

                    cv::putText(frame, infoText,
                        cv::Point(rect.x - 20, rect.y - 20),  // ligeiramente abaixo do texto anterior
                        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);

                    continue; // PULAR para proximo blob
                }

                // AQUI só chega se NÃO está confirmado ainda a moeda
                int currentArea = blobs[i].area;
                int currentPerimeter = blobs[i].perimeter;

                // Adicionar área e perímetro ao histórico para calcular estabilidade
                trackedCoins[matchIndex].areaHistory.push_back(currentArea);
                trackedCoins[matchIndex].perimeterHistory.push_back(currentPerimeter);

                // Manter apenas as últimas STABILITY_THRESHOLD áreas e perímetros
                if (trackedCoins[matchIndex].areaHistory.size() > STABILITY_THRESHOLD) {
                    trackedCoins[matchIndex].areaHistory.erase(
                        trackedCoins[matchIndex].areaHistory.begin());
                    trackedCoins[matchIndex].perimeterHistory.erase(
                        trackedCoins[matchIndex].perimeterHistory.begin());
                }

                // Verificar estabilidade se temos áreas suficientes
                if (trackedCoins[matchIndex].areaHistory.size() >= STABILITY_THRESHOLD) {
                    bool isStable = true;
                    int totalArea = 0;
                    int totalPerimeter = 0;

                    // Calcular área e perímetro médios
                    for (size_t j = 0; j < trackedCoins[matchIndex].areaHistory.size(); j++) {
                        totalArea += trackedCoins[matchIndex].areaHistory[j];
                        totalPerimeter += trackedCoins[matchIndex].perimeterHistory[j];
                    }
                    int avgArea = totalArea / trackedCoins[matchIndex].areaHistory.size();
                    int avgPerimeter = totalPerimeter / trackedCoins[matchIndex].perimeterHistory.size();

                    // Verificar se todas as áreas estão dentro da variação permitida
                    for (int area : trackedCoins[matchIndex].areaHistory) {
                        float variation = abs(area - avgArea) / (float)avgArea;
                        if (variation > MAX_AREA_VARIATION) {
                            isStable = false;
                            break;
                        }
                    }

                    if (isStable) {
                        // CONFIRMAR o tipo com base na área média
                        trackedCoins[matchIndex].typeConfirmed = true;
                        trackedCoins[matchIndex].finalArea = avgArea;
                        trackedCoins[matchIndex].finalPerimeter = avgPerimeter;
                        trackedCoins[matchIndex].finalCircularity = calculate_circularity(avgArea, avgPerimeter);
                        trackedCoins[matchIndex].type = std::string(classify_coin(avgArea, trackedCoins[matchIndex].finalCircularity, centerColor));

                        std::cout << "=== MOEDA CONFIRMADA ===" << std::endl;
                        std::cout << "Frame: " << currentFrame << std::endl;
                        std::cout << "Tipo: " << trackedCoins[matchIndex].type << std::endl;
                        std::cout << "Área final: " << avgArea << std::endl;
                        std::cout << "Perímetro final: " << avgPerimeter << std::endl;
                        std::cout << "Circularidade final: " << trackedCoins[matchIndex].finalCircularity << std::endl;
                        std::cout << "Histórico de áreas: ";
                        for (int area : trackedCoins[matchIndex].areaHistory) {
                            std::cout << area << " ";
                        }
                        std::cout << std::endl;
                        std::cout << "Histórico de perímetros: ";
                        for (int perimeter : trackedCoins[matchIndex].perimeterHistory) {
                            std::cout << perimeter << " ";
                        }
                        std::cout << std::endl;
                        std::cout << "========================" << std::endl;
                    }
                    else {
                        // Resetar o histórico se não estável
                        trackedCoins[matchIndex].areaHistory.clear();
                        trackedCoins[matchIndex].perimeterHistory.clear();
                        trackedCoins[matchIndex].areaHistory.push_back(currentArea);
                        trackedCoins[matchIndex].perimeterHistory.push_back(currentPerimeter);
                    }
                }

                // AZUL - moeda ainda em análise
                cv::Vec3b Blue(255, 0, 0);
                drawRectangleManual(frame, rect, Blue);

                float current_circularity = calculate_circularity(currentArea, currentPerimeter);
                char circularity_str[10];
                sprintf(circularity_str, "%.2f", current_circularity);
                cv::putText(frame, classify_coin(currentArea, current_circularity, centerColor),
                    cv::Point(rect.x + 25, rect.y - 40),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 1);

                // Texto das informações (embaixo do texto do tipo da moeda)
                std::string infoText =
                    "(A:" + std::to_string(currentArea) +
                    " P:" + std::to_string(currentPerimeter) +
                    " C:" + circularity_str +
                    " " + std::to_string(trackedCoins[matchIndex].areaHistory.size()) +
                    "/" + std::to_string(STABILITY_THRESHOLD) + ")";

                cv::putText(frame, infoText,
                    cv::Point(rect.x - 45, rect.y - 20),  // Aqui: 15 pixels abaixo do topo do retângulo
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);
            }
            else {
                // Nova moeda detetada
                CoinTrack newCoin;
                newCoin.bbox = rect;
                newCoin.type = "A Carregar...";
                newCoin.firstSeenFrame = currentFrame;
                newCoin.lastSeenFrame = currentFrame;
                newCoin.counted = false;
                newCoin.typeConfirmed = false;
                newCoin.finalArea = 0;
                newCoin.finalPerimeter = 0;
                newCoin.matched_this_frame = true;
                newCoin.areaHistory.push_back(blobs[i].area);
                newCoin.perimeterHistory.push_back(blobs[i].perimeter);

                trackedCoins.push_back(newCoin);

                // VERMELHO - nova moeda 
                cv::Vec3b Red(0, 0, 255);
                drawRectangleManual(frame, rect, Red);

                float new_circularity = calculate_circularity(blobs[i].area, blobs[i].perimeter);
                char circularity_str[10];
                sprintf(circularity_str, "%.2f", new_circularity);
                std::string text = "NOVA (A:" + std::to_string(blobs[i].area) +
                    " P:" + std::to_string(blobs[i].perimeter) +
                    " C:" + circularity_str + ")";
                cv::putText(frame, text, cv::Point(rect.x + 5, rect.y - 10),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
            }
        }

        // Contar moedas confirmadas que pficaram tempo suficiente
        for (auto& coin : trackedCoins) {
            if (!coin.counted &&
                coin.typeConfirmed &&
                currentFrame - coin.firstSeenFrame >= FRAME_THRESHOLD) {

                coinCount[coin.type]++;
                coin.counted = true;
                TotalCoins++;

                std::cout << ">>> MOEDA CONTADA: " << coin.type
                    << " (Total: " << coinCount[coin.type]
                    << ", Área: " << coin.finalArea
                    << ", Perímetro: " << coin.finalPerimeter
                    << ", Circularidade: " << coin.finalCircularity << ")" << std::endl;
            }
        }

        free(blobs);
    }

    // Mostrar contagem de moedas
    int y_pos = 150;
    for (const auto& pair : coinCount) {
        if (pair.second > 0) {
            std::string countText = pair.first + ": " + std::to_string(pair.second);
            cv::putText(frame, countText, cv::Point(20, y_pos), cv::FONT_HERSHEY_SIMPLEX, 0.8,
                cv::Scalar(255, 255, 255), 2);
            y_pos += 25;
        }
    }

    std::string totalText = "TOTAL: " + std::to_string(TotalCoins);
    cv::putText(frame, totalText, cv::Point(20, y_pos + 10), cv::FONT_HERSHEY_SIMPLEX, 0.9,
        cv::Scalar(0, 255, 255), 2); // Amarelo

    // Remover moedas antigas não vistas
    const int FORGET_THRESHOLD = 70;
    trackedCoins.erase(
        std::remove_if(trackedCoins.begin(), trackedCoins.end(),
            [currentFrame, FORGET_THRESHOLD](const CoinTrack& coin) {
                return currentFrame - coin.lastSeenFrame > FORGET_THRESHOLD;
            }),trackedCoins.end());

    // Liberar memoria
    for (int i = 0; i < 10; i++) {
        vc_image_free(image[i]);
    }
}