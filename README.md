# Projeto Minecraft Character OpenGL

## Descrição
Este projeto é uma implementação 3D interativa de um personagem no estilo Minecraft usando OpenGL. O projeto permite controlar um personagem em um ambiente 3D com diferentes skins, animações e interações com o ambiente.

## Funcionalidades

### Sistema de Movimentação
  - **Movimentação Básica**:
  - Teclas minúsculas (w, a, s, d): Movimento normal
  - Teclas maiúsculas (W, A, S, D): Movimento correndo
  - A movimentação inclui animações sincronizadas de braços e pernas
  - Efeito de bobbing da câmera durante o movimento

- **Detalhes da Animação**:
  - Braços e pernas se movem em sincronia durante a caminhada
  - Velocidade de movimento:
    - Andando: 0.25 unidades por frame
    - Correndo: 0.5 unidades por frame
  - Amplitude do movimento:
    - Braços e pernas oscilam entre -45° e 45°
    - Velocidade de oscilação aumenta durante a corrida
  - Sistema de desaceleração suave ao parar

- **Efeitos Visuais**:
  - Camera bobbing (oscilação vertical da câmera) durante o movimento
  - Animação fluida de transição entre estados (parado/andando/correndo)
  - Rotação do personagem de acordo com a direção do movimento

### Controles da Câmera
- **Mouse (botão esquerdo)**: Rotação da câmera
- **Scroll do mouse**: Zoom in/out

### Controles de Iluminação
- **T/t**: Ativar/desativar luz da tocha
- **Teclas numéricas**:
  - **1/2**: Mover luz para esquerda/direita
  - **3/5**: Mover luz para frente/trás
  - **0/8**: Mover luz para baixo/cima

### Menu de Contexto (Botão Direito)
- **Hora/Clima**: Altera a iluminação do ambiente
  - Dia
  - Entardecer
  - Noite
- **Skin**: Troca a aparência do personagem
  - Herobrine
  - Zoro
  - Venom
- **Reset**: Reinicia todas as posições e animações

## Características Técnicas
- Implementação em C++ com OpenGL
- Sistema de iluminação dinâmica
- Texturas personalizadas
- Animações fluidas
- Sistema de câmera em terceira pessoa
- Colisão básica com objetos do cenário

## Requisitos
- OpenGL
- GLUT/FreeGLUT
- Compilador C++
- Biblioteca STB Image (incluída no projeto)

## Estrutura do Projeto

Projeto CG
│
├── projetos
│   └── aula.cpp
│
└── images
├── Herobrine
├── Monark
├── Venom
└── Outros 

## Como Executar
1. Certifique-se de ter todas as dependências instaladas
2. Compile o arquivo aula.cpp
3. Execute o programa gerado
4. Use o botão direito do mouse para acessar o menu de opções

## Observações
- O projeto utiliza texturas personalizadas para cada skin
- O sistema de iluminação simula diferentes períodos do dia
- As animações são sincronizadas com o movimento do personagem

