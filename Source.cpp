//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//           INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE
//                          2024/2025
//             ENGENHARIA DE SISTEMAS INFORMÁTICOS
//           VISÃO POR COMPUTADOR - TRABALHO PRÁTICO
//
//                       [ GRUPO 26 ]
//					 FICHEIRO - SOURCE.CPP
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <cmath>
#include <vector>
#include <map>

#include "coin_utils.h"
#include "coin_detector.h"

int main(void) {
    // Inicializar contagem de moedas
    coinCount["1 centimo"] = 0;
    coinCount["2 centimos"] = 0;
    coinCount["5 centimos"] = 0;
    coinCount["10 centimos"] = 0;
    coinCount["20 centimos"] = 0;
    coinCount["50 centimos"] = 0;
    coinCount["1 euro"] = 0;
    coinCount["2 euros"] = 0;
    coinCount["Total Moedas"] = 0;

    // Vídeo
    char videofile[20] = "video1.mp4";
    cv::VideoCapture capture;
    struct
    {
        int width, height;
        int ntotalframes;
        int fps;
        int nframe;
    } video;
    std::string str;
    int key = 0;

    /* Leitura de vídeo de um ficheiro */
    capture.open(videofile);

    /* Verifica se foi possível abrir o ficheiro de vídeo */
    if (!capture.isOpened())
    {
        std::cerr << "Erro ao abrir o ficheiro de vídeo!\n";
        return 1;
    }

    /* Número total de frames no vídeo */
    video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
    /* Frame rate do vídeo */
    video.fps = (int)capture.get(cv::CAP_PROP_FPS);
    /* Resolução do vídeo */
    video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

    /* Cria uma janela para exibir o vídeo */
    cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

    /* Inicia o timer */
    vc_timer();

    cv::Mat frame;
    while (key != 'q') {
        /* Leitura de uma frame do vídeo */
        capture.read(frame);

        /* Verifica se conseguiu ler a frame */
        if (frame.empty()) break;

        /* Número da frame a processar */
        video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

        // Processamento da frame para detecção de moedas
        detect_coins_in_frame(frame, video.nframe);

        /* Exemplo de inserção texto na frame */
        str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
        cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
        cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
        cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

        /* Exibe a frame */
        cv::imshow("VC - VIDEO", frame);

        /* Sai da aplicação, se o utilizador premir a tecla 'q' */
        key = cv::waitKey(10);
    }

    /* Para o timer e exibe o tempo decorrido */
    vc_timer();

    /* Exibe a contagem final de moedas */
    std::cout << "\n-- CONTAGEM FINAL DE MOEDAS --" << std::endl;
    for (const auto& pair : coinCount) {
        if (pair.second > 0) { // Só mostrar tipos que foram detectados
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
    }

    /* Fecha a janela */
    cv::destroyWindow("VC - VIDEO");

    /* Fecha o ficheiro de vídeo */
    capture.release();

    return 0;
}