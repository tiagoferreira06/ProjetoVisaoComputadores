# 🔍 Projeto de Visão por Computador: Detecção e Quantificação de Moedas 💰

## 📝 Descrição
Este projeto foi desenvolvido como trabalho prático para a disciplina de Visão por Computador do curso de Engenharia de Sistemas Informáticos do Instituto Politécnico do Cávado e do Ave (IPCA), ano letivo 2024/2025. O sistema é capaz de identificar e quantificar moedas em vídeos, determinando a quantidade e o valor monetário total.

## 👨‍💻 Autor
- **Tiago Nunes Ferreira**
- Grupo 26
- Engenharia de Sistemas Informáticos
- IPCA - Instituto Politécnico do Cávado e do Ave

## ✨ Funcionalidades
- 🎯 Identificação automática de moedas em vídeos (formato MP4)
- 🏷️ Classificação do tipo de moeda (1, 2, 5, 10, 20, 50 cêntimos e 1, 2 euros)
- 🔢 Contagem total de moedas por tipo
- 📏 Cálculo da área e perímetro de cada moeda detectada
- 🖼️ Visualização em tempo real da localização das moedas (com caixa delimitadora)
- ⚪ Exibição do centro de gravidade de cada moeda
- 💶 Indicação do tipo de moeda na interface

## 🛠️ Requisitos Técnicos
- C/C++
- OpenCV (versão utilizada para desenvolvimento: 4.x)
- Sistema de compilação compatível (Visual Studio, GCC, etc.)

## 📂 Estrutura do Projeto
- `Source.cpp`: Arquivo principal contendo o ponto de entrada da aplicação
- `coin_utils.h`: Utilitários para manipulação de moedas e métricas
- `coin_detector.h`: Implementação dos algoritmos de detecção e classificação de moedas

## 🧠 Técnicas Implementadas
- 🎨 Segmentação por tonalidade e brilho
- 🧹 Filtragem para redução de ruído
- 🔍 Análise de componentes conectados
- 📊 Extração de características (área, perímetro, circularidade)
- 🏅 Classificação baseada em características físicas das moedas
- 🎯 Rastreamento de objetos entre frames

## ▶️ Como Executar
1. Certifique-se de ter o OpenCV instalado e configurado corretamente
2. Compile o projeto usando seu compilador C++ preferido
3. Coloque o arquivo de vídeo (`video1.mp4` ou `video2.mp4`) no mesmo diretório do executável
4. Execute o programa

## 🎮 Controles
- Pressione 'q' para encerrar a aplicação

## 📝 Observações
- O sistema foi otimizado para vídeos com resolução de 1280x720 e taxa de 30 fps
- A detecção pode variar de acordo com as condições de iluminação e ângulo da câmera
- O programa exibe estatísticas finais ao encerrar a execução

## 🎓 Avaliação e Contexto Acadêmico
Este trabalho foi desenvolvido como parte da avaliação da disciplina de Visão por Computador, com o objetivo de aplicar conceitos de processamento e análise de imagem aprendidos em aula.

---
