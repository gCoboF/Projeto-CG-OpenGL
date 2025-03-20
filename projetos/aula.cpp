#include <GL/freeglut.h>
#include <GL/gl.h>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint idsTextura[50];

#define NUM_SKINS 3
#define TEXTURES_PER_SKIN 26

GLuint skin1Texturas[TEXTURES_PER_SKIN];
GLuint skin2Texturas[TEXTURES_PER_SKIN];
GLuint skin3Texturas[TEXTURES_PER_SKIN];

GLuint* currentSkin = skin1Texturas; // Skin inicial

float distanciaCamera = 10.0f;
float anguloCamera = 0.0f; // Define a câmera inicialmente de frente

const float ZOOM_STEP = 0.5f;           // Passo de zoom por evento de scroll
const float MIN_DISTANCE = 2.0f;        // Distância mínima da câmera
const float MAX_DISTANCE = 50.0f;       // Distância máxima da câmera

float anguloBracos = 0.0f;
float anguloPernas = 0.0f;
bool aumentandoBracos = false;
bool aumentandoPernas = false;

float direcaoPersonagem = 0.0f; // Ângulo inicial do personagem (frente para a tela)
bool andando = false;
bool correndo = false;
bool desacelerando = false; // Indica se o personagem está desacelerando

float posX = 0.0f;  // Posição do personagem no eixo X
float posZ = 0.0f;  // Posição do personagem no eixo Z

bool crouching = false;

int ultimoX, ultimoY;
bool botaoEsquerdoPressionado = false;

// Variáveis para rotação da cabeça
float anguloCabecaY = 0.0f; // Rotação da cabeça no eixo Y (esquerda/direita)
float anguloCabecaX = 0.0f; // Rotação da cabeça no eixo X (cima/baixo)
const float LIMITE_CABECA_X = 20.0f; // Limite máximo para rotação vertical da cabeça
const float INCREMENTO_CABECA = 5.0f; // Incremento de rotação por pressionamento de tecla
float anguloPernaManualEsquerda = 0.0f; 
const float LIMITE_PERNA_MANUAL_ESQUERDA = 90.0f; 
const float INCREMENTO_MANUAL_PERNA_ESQUERDA = 5.0f; 

float anguloPernaManualDireita = 0.0f; 
const float LIMITE_PERNA_MANUAL_DIREITA = 90.0f; 
const float INCREMENTO_MANUAL_PERNA_DIREITA = 5.0f; 

float anguloBracoManualEsquerda = 0.0f; 
const float LIMITE_BRACO_MANUAL_ESQUERDA = 180.0f; 
const float INCREMENTO_MANUAL_BRACO_ESQUERDA = 5.0f; 

float anguloBracoManualDireita = 0.0f; 
const float LIMITE_BRACO_MANUAL_DIREITA = 180.0f; 
const float INCREMENTO_MANUAL_BRACO_DIREITA = 5.0f; 

float lightPosX = 6.0f;
float lightPosY = 8.0f; // Altura fixa para simular o sol no céu
float lightPosZ = -9.0f;
float velocidadeMovimento = 1.0f; // Valor inicial pode ser ajustado conforme necessário

// Cores de fundo (RGBA)
float bgColor[4] = {0.5f, 0.7f, 1.0f, 1.0f}; // Azul claro padrão (Dia)

// Cores da luz (RGBA)
float lightAmbient[4]  = {0.4f, 0.4f, 0.4f, 1.0f};
float lightDiffuse[4]  = {1.0f, 1.0f, 1.0f, 1.0f};
float lightSpecular[4] = {1.0f, 1.0f, 1.0f, 1.0f};

// Variáveis para o efeito de bobbing da câmera
float cameraOffsetY = 0.0f;                 // Deslocamento vertical atual da câmera
float cameraOscillationAngle = 0.0f;        // Ângulo de oscilação para calcular o deslocamento
const float CAMERA_OSCILLATION_SPEED = 0.1f; // Velocidade da oscilação (ajuste conforme necessário)
const float CAMERA_OSCILLATION_AMPLITUDE = 0.2f; // Amplitude máxima da oscilação (ajuste conforme necessário)

// Variáveis globais adicionais


// Variável para controlar a visibilidade do retângulo
bool retanguloVisivel = false;

// Identificador para a nova luz
// OpenGL geralmente suporta de GL_LIGHT0 a GL_LIGHT7
#define LIGHT_RETANGULO GL_LIGHT1

GLuint textureID;

void carregarTextura(const char* nomeArquivo, GLuint texturaArray[], int indice)
{
    // Inverter a imagem verticalmente
    stbi_set_flip_vertically_on_load(true);

    int largura, altura, canais;
    unsigned char *dados = stbi_load(nomeArquivo, &largura, &altura, &canais, 0);

    if (!dados) {
        printf("Erro ao carregar a textura: %s\n", nomeArquivo);
        exit(1);
    }

    glGenTextures(1, &texturaArray[indice]);
    glBindTexture(GL_TEXTURE_2D, texturaArray[indice]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Corrigir os parâmetros de wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLenum formato = (canais == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, formato, 
                 largura, altura, 0, formato,
                 GL_UNSIGNED_BYTE, dados);

    stbi_image_free(dados); // Libera memória da imagem
}


void atualizaAnimacao(int value) {
    if (andando || correndo) {
        float velocidade = correndo ? 6.0f : 4.0f;

        if (aumentandoBracos) {
            anguloBracos += velocidade;
            if (anguloBracos > 45.0f) aumentandoBracos = false;
        } else {
            anguloBracos -= velocidade;
            if (anguloBracos < -45.0f) aumentandoBracos = true;
        }

        if (aumentandoPernas) {
            anguloPernas += velocidade;
            if (anguloPernas > 45.0f) aumentandoPernas = false;
        } else {
            anguloPernas -= velocidade;
            if (anguloPernas < -45.0f) aumentandoPernas = true;
        }

        // Atualiza o ângulo de oscilação da câmera
        cameraOscillationAngle += CAMERA_OSCILLATION_SPEED;
        cameraOffsetY = sin(cameraOscillationAngle) * CAMERA_OSCILLATION_AMPLITUDE;

    } else if (desacelerando) {
        // Reduz gradualmente os ângulos dos braços e pernas até 0
        if (anguloBracos > 0.0f) {
            anguloBracos -= 2.0f;
            if (anguloBracos < 0.0f) anguloBracos = 0.0f;
        } else if (anguloBracos < 0.0f) {
            anguloBracos += 2.0f;
            if (anguloBracos > 0.0f) anguloBracos = 0.0f;
        }

        if (anguloPernas > 0.0f) {
            anguloPernas -= 2.0f;
            if (anguloPernas < 0.0f) anguloPernas = 0.0f;
        } else if (anguloPernas < 0.0f) {
            anguloPernas += 2.0f;
            if (anguloPernas > 0.0f) anguloPernas = 0.0f;
        }

        // Se ambos os ângulos forem 0, desativa a desaceleração
        if (anguloBracos == 0.0f && anguloPernas == 0.0f) {
            desacelerando = false;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, atualizaAnimacao, 0);
}

void teclado(unsigned char key, int x, int y) {
    float velocidade = correndo ? 0.5f : 0.25f;  // Velocidade de movimento do personagem

     if (key == 'a' || key == 'A') {  // Movimento para a esquerda
        //anguloBracoManualEsquerda = 0.0f;
        //anguloBracoManualDireita = 0.0f;

        anguloPernaManualEsquerda = 0.0f;
        anguloPernaManualDireita = 0.0f;

        anguloCabecaY = 0.0f;
        anguloCabecaX = 0.0f;

        // Movimento horizontal fixo, enquanto o corpo está a 45 graus para a esquerda
        posX -= velocidade;  // Movimento na direção X (para a esquerda)
        posZ += 0.0f;        // Sem alteração na direção Z

        // Atualiza a direção do personagem para 45 graus para a esquerda
        direcaoPersonagem = -M_PI / 4;  
        
        andando = true;
        correndo = (key == 'A');  // Correndo com a tecla 'A' maiúscula
    } else if (key == 'd' || key == 'D') {  // Movimento para a direita
        //anguloBracoManualEsquerda = 0.0f;
        //anguloBracoManualDireita = 0.0f;

        anguloPernaManualEsquerda = 0.0f;
        anguloPernaManualDireita = 0.0f; 

        anguloCabecaY = 0.0f;
        anguloCabecaX = 0.0f;       

        // Movimento horizontal fixo, enquanto o corpo está a 45 graus para a direita
        posX += velocidade;  // Movimento na direção X (para a direita)
        posZ += 0.0f;        // Sem alteração na direção Z

        // Atualiza a direção do personagem para 45 graus para a direita
        direcaoPersonagem = M_PI / 4;  
        
        andando = true;
        correndo = (key == 'D');  // Correndo com a tecla 'D' maiúscula
    } else if (key == 'w' || key == 'W') {  // Movimento para frente
        direcaoPersonagem = 0.0f;

        //anguloBracoManualEsquerda = 0.0f;
        //anguloBracoManualDireita = 0.0f;

        anguloPernaManualEsquerda = 0.0f;
        anguloPernaManualDireita = 0.0f;

        anguloCabecaY = 0.0f;
        anguloCabecaX = 0.0f;

        posX += velocidade * cos(direcaoPersonagem + M_PI / 2);
        posZ -= velocidade * sin(direcaoPersonagem + M_PI / 2);
        andando = true;
        correndo = (key == 'W');  // Correndo com a tecla 'W' maiúscula
    } else if (key == 's' || key == 'S') {  // Movimento para trás
        direcaoPersonagem = 0.0f;

        //anguloBracoManualEsquerda = 0.0f;
        //anguloBracoManualDireita = 0.0f;

        anguloPernaManualEsquerda = 0.0f;
        anguloPernaManualDireita = 0.0f;

        anguloCabecaY = 0.0f;
        anguloCabecaX = 0.0f;

        posX -= velocidade * cos(direcaoPersonagem + M_PI / 2);
        posZ += velocidade * sin(direcaoPersonagem + M_PI / 2);
        andando = true;
        correndo = (key == 'S');  // Correndo com a tecla 'S' maiúscula
    } else if (key =='c'|| key == 'C'){
        crouching = !crouching;
    }else if (key == 'v' || key == 'V') {  // Rotação manual dos braços
        if (key == 'v') { // minúsculo para um sentido (ex: para trás)
            anguloBracoManualEsquerda -= INCREMENTO_MANUAL_BRACO_ESQUERDA;
            if (anguloBracoManualEsquerda < -LIMITE_BRACO_MANUAL_ESQUERDA)
                anguloBracoManualEsquerda = -LIMITE_BRACO_MANUAL_ESQUERDA;
        }
        else { // maiúsculo para o sentido oposto (ex: para frente)
            anguloBracoManualEsquerda += INCREMENTO_MANUAL_BRACO_ESQUERDA;
            if (anguloBracoManualEsquerda > LIMITE_BRACO_MANUAL_ESQUERDA)
                anguloBracoManualEsquerda = LIMITE_BRACO_MANUAL_ESQUERDA;
        }
    }
    else if (key == 'b' || key == 'B') {  // Rotação manual dos braços
        if (key == 'b') { // minúsculo para um sentido (ex: para trás)
            anguloBracoManualDireita -= INCREMENTO_MANUAL_BRACO_DIREITA;
            if (anguloBracoManualDireita < -LIMITE_BRACO_MANUAL_DIREITA)
                anguloBracoManualDireita = -LIMITE_BRACO_MANUAL_DIREITA;
        }
        else { // maiúsculo para o sentido oposto (ex: para frente)
            anguloBracoManualDireita += INCREMENTO_MANUAL_BRACO_DIREITA;
            if (anguloBracoManualDireita > LIMITE_BRACO_MANUAL_DIREITA)
                anguloBracoManualDireita = LIMITE_BRACO_MANUAL_DIREITA;
        }
    }
    else if (key == 'o' || key == 'O') {  // Rotação manual da perna esquerda
        if (key == 'o') { // minúsculo para um sentido (ex: para trás) 
            anguloPernaManualEsquerda -= INCREMENTO_MANUAL_PERNA_ESQUERDA;
                if (anguloPernaManualEsquerda < -LIMITE_PERNA_MANUAL_ESQUERDA)
                    anguloPernaManualEsquerda = -LIMITE_PERNA_MANUAL_ESQUERDA;
            }
            else { // maiúsculo para o sentido oposto (ex: para frente)
                anguloPernaManualEsquerda += INCREMENTO_MANUAL_PERNA_ESQUERDA;
                if (anguloPernaManualEsquerda > LIMITE_PERNA_MANUAL_ESQUERDA)
                    anguloPernaManualEsquerda = LIMITE_PERNA_MANUAL_ESQUERDA;
            }
    }
    else if (key == 'p' || key == 'P') {  // Rotação manual da perna direita
        if (key == 'p') { // minúsculo para um sentido (ex: para trás)
            anguloPernaManualDireita -= INCREMENTO_MANUAL_PERNA_DIREITA;
            if (anguloPernaManualDireita < -LIMITE_PERNA_MANUAL_DIREITA)
                anguloPernaManualDireita = -LIMITE_PERNA_MANUAL_DIREITA;
        }
        else { // maiúsculo para o sentido oposto (ex: para frente)
            anguloPernaManualDireita += INCREMENTO_MANUAL_PERNA_DIREITA;
            if (anguloPernaManualDireita > LIMITE_PERNA_MANUAL_DIREITA)
                anguloPernaManualDireita = LIMITE_PERNA_MANUAL_DIREITA;
        }
    }else if (key == '1') { // Mover para a esquerda
        lightPosX -= velocidadeMovimento;
    }
    else if (key == '2') { // Mover para a direita
        lightPosZ += velocidadeMovimento;
    }
    else if (key == '3') { // Mover para direita
        lightPosX += velocidadeMovimento;
    }
    else if (key == '5') { // Mover para frente
        lightPosZ -= velocidadeMovimento;
    } 
    else if (key == '0') { // Mover para baixo
        lightPosY -= velocidadeMovimento;
    }
    else if (key == '8') { // Mover para baixo
        lightPosY += velocidadeMovimento;
    }else if (key == 't' || key == 'T') {  // Detecta a tecla 'T' minúscula ou maiúscula
        retanguloVisivel = !retanguloVisivel;  // Alterna o estado de visibilidade

        if (retanguloVisivel) {
            // Ativa a nova luz
            glEnable(LIGHT_RETANGULO); // Usaremos GL_LIGHT1 para o retângulo
        } else {
            // Desativa a nova luz
            glDisable(LIGHT_RETANGULO);
        }
    }


    glutPostRedisplay();
}

void tecladoUp(unsigned char key, int x, int y) {
    if (key == 'a' || key == 'A' || key == 'd' || key == 'D' || key == 's' || key == 'S' || key == 'W' || key == 'w') {
        andando = false;
        correndo = false;
        desacelerando = true; // Ativa a desaceleração
    } 
    glutPostRedisplay();
}

void tecladoEspecial(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_LEFT:
            anguloCabecaY -= INCREMENTO_CABECA; // Rotaciona a cabeça 5 graus para a esquerda
            if (anguloCabecaY < -90.0f)
                anguloCabecaY = -90.0f; // Limita a rotação à esquerda
            break;
        case GLUT_KEY_RIGHT:
            anguloCabecaY += INCREMENTO_CABECA; // Rotaciona a cabeça 5 graus para a direita
            if (anguloCabecaY > 90.0f)
                anguloCabecaY = 90.0f; // Limita a rotação à direita
            break;
        case GLUT_KEY_UP:
            anguloCabecaX -= INCREMENTO_CABECA; // Inclina a cabeça 5 graus para baixo (inversão)
            if (anguloCabecaX < -LIMITE_CABECA_X)
                anguloCabecaX = -LIMITE_CABECA_X; // Limita a inclinação para baixo
            break;
        case GLUT_KEY_DOWN:
            anguloCabecaX += INCREMENTO_CABECA; // Inclina a cabeça 5 graus para cima (inversão)
            if (anguloCabecaX > LIMITE_CABECA_X)
                anguloCabecaX = LIMITE_CABECA_X; // Limita a inclinação para cima
            break;
    }

    glutPostRedisplay(); // Atualiza a tela para refletir as mudanças
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            botaoEsquerdoPressionado = true;
            ultimoX = x;
            ultimoY = y;
        } else {
            botaoEsquerdoPressionado = false;
        }
    }else if (button == 3) { // Scroll para cima (Zoom in)
        distanciaCamera -= ZOOM_STEP;
        if (distanciaCamera < MIN_DISTANCE)
            distanciaCamera = MIN_DISTANCE;
        glutPostRedisplay();
    }
    else if (button == 4) { // Scroll para baixo (Zoom out)
        distanciaCamera += ZOOM_STEP;
        if (distanciaCamera > MAX_DISTANCE)
            distanciaCamera = MAX_DISTANCE;
        glutPostRedisplay();
    }
}

void arrastaMouse(int x, int y) {
    if (botaoEsquerdoPressionado) {
        int dx = x - ultimoX;
        anguloCamera += dx * 0.5f; // Ajuste o 0.5f conforme a sensibilidade desejada
        ultimoX = x;
        ultimoY = y;
        glutPostRedisplay();
    }
}

void desenhaChao(float tamanho) {
    glEnable(GL_TEXTURE_2D);

    // Desabilita o blending, caso esteja ativado sem necessidade
    glDisable(GL_BLEND);

    glColor3f(0.5f, 0.7f, 0.5f);
    // Supondo que a textura do chão tenha índice 0
    glBindTexture(GL_TEXTURE_2D, idsTextura[24]);

    glBegin(GL_QUADS);

    // Chão (plano XZ)
    float repeticao = 60.0f;  // Define quantas vezes a textura vai se repetir ao longo do chão

    // Calcula as coordenadas de textura de forma proporcional
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-tamanho, -2.0f, -tamanho);  // Ponto inferior esquerdo
    glTexCoord2f(repeticao, 0.0f); glVertex3f(tamanho, -2.0f, -tamanho);   // Ponto inferior direito
    glTexCoord2f(repeticao, repeticao); glVertex3f(tamanho, -2.0f, tamanho);    // Ponto superior direito
    glTexCoord2f(0.0f, repeticao); glVertex3f(-tamanho, -2.0f, tamanho);   // Ponto superior esquerdo

    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void desenhaCubo(GLfloat x, GLfloat y, GLfloat z, int texturaIndices[6], GLuint* texturaArray) {
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f); // Garante que as texturas não sejam tingidas

    for(int face = 0; face < 6; ++face) {
        glBindTexture(GL_TEXTURE_2D, texturaArray[texturaIndices[face]]);
        glBegin(GL_QUADS);
            switch(face) {
                case 0: // Frente
                    glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, -y, z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, -y, z);
                    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f(-x, y, z);
                    break;
                case 1: // Trás
                    glTexCoord2f(0.0f, 0.0f); glVertex3f(x, -y, -z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f(-x, -y, -z);
                    glTexCoord2f(1.0f, 1.0f); glVertex3f(-x, y, -z);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, -z);
                    break;
                case 2: // Esquerda
                    glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, -y, -z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f(-x, -y, z);
                    glTexCoord2f(1.0f, 1.0f); glVertex3f(-x, y, z);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f(-x, y, -z);
                    break;
                case 3: // Direita
                    glTexCoord2f(0.0f, 0.0f); glVertex3f(x, -y, -z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, -y, z);
                    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y, -z);
                    break;
                case 4: // Cima
                    glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, y, -z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, -z);
                    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f(-x, y, z);
                    break;
                case 5: // Embaixo
                    glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, -y, -z);
                    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, -y, -z);
                    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, -y, z);
                    glTexCoord2f(0.0f, 1.0f); glVertex3f(-x, -y, z);
                    break;
            }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
}


void desenhaPersonagem()
{

    if (crouching)
    {
        glTranslatef(0.0f, 0.0f, -0.25f);
    }

    // Perna Direita
    glPushMatrix();
    glTranslatef(-0.25f, -1.25f, 0.0f);
    glTranslatef(0.0f, 1.0f, 0.0f);
    glRotatef(anguloPernas + anguloPernaManualDireita, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -1.0f, 0.0f);
    int texturaPernaDireita[6] = {18, 19, 21, 20, 22, 23};
    desenhaCubo(0.25f,0.75f, 0.25f, texturaPernaDireita, currentSkin);
    glPopMatrix();

    // Perna Esquerda
    glPushMatrix();
    glTranslatef(0.25f, -1.25f, 0.0f);
    glTranslatef(0.0f, 1.0f, 0.0f);
    glRotatef(-anguloPernas + anguloPernaManualEsquerda, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -1.0f, 0.0f);
    int texturaPernaEsquerda[6] = {18, 19, 20, 21, 22, 23};
    desenhaCubo(0.25f, 0.75f, 0.25f, texturaPernaEsquerda, currentSkin);
    glPopMatrix();

    //glTranslatef(0.0f, 0.0f, 0.0f); // reseta translação


    if (crouching)
    {
        glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, -0.25f, 0.25f);

    }

    // Cabeça
    glPushMatrix();
        glTranslatef(0.0f, 1.5f, 0.0f);
        
        // Rotação para crouch no eixo X
        if (crouching) {
            glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);
        }

        // Rotação no eixo Y para girar a cabeça (esquerda/direita)
        glRotatef(anguloCabecaY, 0.0f, 1.0f, 0.0f); // Aplica a rotação Y
        
        // Rotação no eixo X para inclinar a cabeça (cima/baixo)
        glRotatef(anguloCabecaX, 1.0f, 0.0f, 0.0f); // Aplica a rotação X
        
        int texturaCabeca[6] = {0, 1, 2, 3, 4, 5};
        desenhaCubo(0.5f, 0.5f, 0.5f, texturaCabeca, currentSkin);
    glPopMatrix();


    // Tronco
    glPushMatrix();
    glTranslatef(0.0f, 0.25f, 0.0f);
    int texturaTronco[6] = {6, 7, 8, 9, 10, 11};
    desenhaCubo(0.5f, 0.75f, 0.25f, texturaTronco, currentSkin);
    glPopMatrix();

    if(crouching){
        glTranslatef(0.0f, 0.0f, 0.15f);
    }
    // Braço esquerdo
    glPushMatrix();
    glTranslatef(-0.75f, 0.25f, 0.0f);
    glTranslatef(0.0f, 0.75f, 0.0f);
    glRotatef(-anguloBracos + anguloBracoManualEsquerda, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.75f, 0.0f);
    int texturaBracoEsquerdo[6] = {12, 13, 14, 15, 16, 17};
    desenhaCubo(0.25f, 0.75f, 0.25f, texturaBracoEsquerdo, currentSkin);
    glPopMatrix();

    // Braço direito
    glPushMatrix();
    glTranslatef(0.75f, 0.25f, 0.0f);
    glTranslatef(0.0f, 0.75f, 0.0f);
    glRotatef(anguloBracos + anguloBracoManualDireita, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -0.75f, 0.0f);
    // Remova ou comente esta linha
    // glColor3f(1.0f, 1.0f, 1.0f);
    int texturaBracoDireito[6] = {12, 13, 15, 14, 16, 17};
    desenhaCubo(0.25f, 0.75f, 0.25f, texturaBracoDireito, currentSkin);

     // Desenhar o retângulo se estiver visível
    if (retanguloVisivel) {
        glPushMatrix();
            // Posicionar na extremidade do braço
            glTranslatef(0.0f, -0.55f, 0.25f); 
            // Orientar o retângulo se necessário
            glRotatef(0.0f, 0.0f, 1.0f, 0.0f); 

            // Definir as texturas para o retângulo
            // Vamos usar a mesma textura em todas as faces
            int texturaRetangulo[6] = {27, 28, 26, 26, 29, 29};
            
            desenhaCubo(0.15f, 0.15f, 0.6f, texturaRetangulo, idsTextura);

            // Atualizar a posição da luz para a posição do retângulo
            // Precisamos obter a posição atual do retângulo no mundo
            // Para isso, podemos usar as transformações aplicadas até agora
            GLfloat light1_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glGetFloatv(GL_MODELVIEW_MATRIX, light1_position);
            // Extrair a posição da matriz de modelo
            light1_position[0] = light1_position[12];
            light1_position[1] = light1_position[13];
            light1_position[2] = light1_position[14];
            light1_position[3] = 1.0f; // Luz pontual

            // Atualizar a posição da luz1
            glLightfv(LIGHT_RETANGULO, GL_POSITION, light1_position);
        glPopMatrix();
    }

    glPopMatrix();

    
}

// Função para resetar todas as animações e posições do personagem
void resetAnimations() {
    // Resetar posição do personagem
    posX = 0.0f;
    posZ = 0.0f;

    // Resetar ângulos de animação automática
    anguloBracos = 0.0f;
    anguloPernas = 0.0f;
    aumentandoBracos = false;
    aumentandoPernas = false;

    // Resetar ângulos de rotação manual
    anguloBracoManualDireita = 0.0f;
    anguloBracoManualEsquerda = 0.0f;
    anguloPernaManualEsquerda = 0.0f;
    anguloPernaManualDireita = 0.0f;

    // Resetar ângulos da cabeça
    anguloCabecaY = 0.0f;
    anguloCabecaX = 0.0f;

    // Resetar estados de movimento
    andando = false;
    correndo = false;
    desacelerando = false;
    crouching = false;

    // Resetar ângulos da câmera
    anguloCamera = 0.0f;
    direcaoPersonagem = 0.0f;

    // Resetar posição da luz
    lightPosX = 6.0f;
    lightPosY = 8.0f;
    lightPosZ = -9.0f;

    distanciaCamera = 10.0f;
}

// Enumeração para as opções do menu
enum MenuOptions {
    MENU_RESET = 0,
    MENU_DIA,
    MENU_ENTARDECER,
    MENU_NOITE
};

// Enumeração para as opções de texturas
enum TextureOptions {
    TEXTURE_SKIN_1 = 100,
    TEXTURE_SKIN_2,
    TEXTURE_SKIN_3,
    // Adicione mais opções conforme necessário
};

// Função de callback para o menu
void menu(int option) {
    switch(option) {
        case MENU_RESET:
            resetAnimations();
            break;
        case MENU_DIA:
            // Configura para Dia
            bgColor[0] = 0.5f;
            bgColor[1] = 0.7f;
            bgColor[2] = 1.0f;
            bgColor[3] = 1.0f;

            lightAmbient[0] = 0.4f;
            lightAmbient[1] = 0.4f;
            lightAmbient[2] = 0.4f;
            lightAmbient[3] = 1.0f;

            lightDiffuse[0] = 1.0f;
            lightDiffuse[1] = 1.0f;
            lightDiffuse[2] = 1.0f;
            lightDiffuse[3] = 1.0f;

            lightSpecular[0] = 1.0f;
            lightSpecular[1] = 1.0f;
            lightSpecular[2] = 1.0f;
            lightSpecular[3] = 1.0f;
            break;
        case MENU_ENTARDECER:
            // Configura para Entardecer
            bgColor[0] = 1.0f;   // Vermelho total
            bgColor[1] = 0.6f;   // Laranja mais suave
            bgColor[2] = 0.4f;   // Amarelo pastel
            bgColor[3] = 1.0f;

            lightAmbient[0] = 0.3f;
            lightAmbient[1] = 0.2f;
            lightAmbient[2] = 0.1f;
            lightAmbient[3] = 1.0f;

            lightDiffuse[0] = 1.0f;
            lightDiffuse[1] = 0.6f;
            lightDiffuse[2] = 0.3f;
            lightDiffuse[3] = 1.0f;

            lightSpecular[0] = 1.0f;
            lightSpecular[1] = 0.6f;
            lightSpecular[2] = 0.3f;
            lightSpecular[3] = 1.0f;
            break;
        case MENU_NOITE:
            // Configura para Noite
            bgColor[0] = 0.0f;
            bgColor[1] = 0.0f;
            bgColor[2] = 0.2f;   // Azul escuro
            bgColor[3] = 1.0f;

            lightAmbient[0] = 0.1f;
            lightAmbient[1] = 0.1f;
            lightAmbient[2] = 0.2f;
            lightAmbient[3] = 1.0f;

            lightDiffuse[0] = 0.2f;
            lightDiffuse[1] = 0.2f;
            lightDiffuse[2] = 0.5f;
            lightDiffuse[3] = 1.0f;

            lightSpecular[0] = 0.2f;
            lightSpecular[1] = 0.2f;
            lightSpecular[2] = 0.5f;
            lightSpecular[3] = 1.0f;
            break;
        case TEXTURE_SKIN_1:
            currentSkin = skin1Texturas;
            break;
        case TEXTURE_SKIN_2:
            currentSkin = skin2Texturas;
            break;
        case TEXTURE_SKIN_3:
            currentSkin = skin3Texturas;
            break;
        default:
            break;
    }
    glutPostRedisplay(); // Redesenha a cena após a seleção
}

void display() {
    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);  // Define a cor de fundo inicial
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float rad = anguloCamera * (M_PI / 180.0f);
    float camX = posX + distanciaCamera * sin(rad);
    float camZ = posZ + distanciaCamera * cos(rad);

    // Aplica o deslocamento vertical da câmera
    float eyeY = 2.0f + cameraOffsetY; // Posição Y da câmera
    float centerY = 1.0f + cameraOffsetY; // Posição Y do ponto onde a câmera está olhando

    gluLookAt(camX, eyeY, camZ,
            posX, centerY, posZ,
            0.0, 1.0, 0.0);

    // Desenha o chão (plano XZ)
    desenhaChao(100.0f);  // Tamanho do chão (grande o suficiente para o personagem se mover)

    // Aplica a direção do personagem
    glPushMatrix();
    glTranslatef(posX, 0.0f, posZ);  // Atualiza a posição do personagem
    glRotatef(direcaoPersonagem * 180.0f / M_PI, 0.0f, 1.0f, 0.0f);
    desenhaPersonagem();
    glPopMatrix();

    // Atualiza as propriedades da luz com base nas variáveis
    glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    GLfloat light_position[] = { lightPosX, lightPosY, lightPosZ, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    // Desenha o cubo de luz atrás e acima do personagem
    glPushMatrix();
    glTranslatef(lightPosX, lightPosY, lightPosZ - 10.0f); // 5 unidades acima e 5 unidades atrás
    int texturaLuz[6] = {25, 25, 25, 25, 25, 25}; // Todas as faces usam a textura da luz
    desenhaCubo(0.2f, 0.2f, 0.2f, texturaLuz, idsTextura); // Tamanho pequeno para a luz
    glPopMatrix();


    glPushMatrix();
        glTranslatef(3.0f, -1.2f, -3.0f); // Posição fixa no mundo
        int texturaCubo1[6] = {30, 30, 31, 31, 32, 32}; // Todas as faces usam a textura 29
        desenhaCubo(1.0f, 1.0f, 1.0f, texturaCubo1, idsTextura);
    glPopMatrix();

    // **Cubo 2**
    glPushMatrix();
        glTranslatef(5.0f, -1.2f, -3.0f); // Outra posição fixa no mundo
        int texturaCubo2[6] = {33, 34, 34, 34, 35, 35}; // Todas as faces usam a textura 29
        desenhaCubo(1.0f, 1.0f, 1.0f, texturaCubo2, idsTextura);
    glPopMatrix();


    glutSwapBuffers();
}

void inicializa() {
    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);  // Define a cor de fundo inicial
    glEnable(GL_DEPTH_TEST);
   
   // Habilita o sistema de iluminação
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); // Usaremos a luz GL_LIGHT0
    glEnable(GL_NORMALIZE); // Normaliza as normais

    // Define as propriedades da luz GL_LIGHT0
    GLfloat light_ambient[]  = { 0.4f, 0.4f, 0.4f, 1.0f };   // Luz ambiente
    GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };   // Iluminação direta das superfícies
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };   // Cria reflexos brilhantes nas superfícies

    GLfloat light_position[] = { lightPosX, lightPosY, lightPosZ, 1.0f }; // w=1.0f para luz pontual

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    // Define as propriedades do material padrão
    GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_shininess[]  = { 50.0f };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // Configurações para GL_LIGHT1 (Luz do retângulo)
    GLfloat light1_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat light1_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f }; // Luz branca
    GLfloat light1_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light1_position[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Será atualizada dinamicamente

    glLightfv(LIGHT_RETANGULO, GL_AMBIENT,  light1_ambient);
    glLightfv(LIGHT_RETANGULO, GL_DIFFUSE,  light1_diffuse);
    glLightfv(LIGHT_RETANGULO, GL_SPECULAR, light1_specular);
    glLightfv(LIGHT_RETANGULO, GL_POSITION, light1_position);

    glEnable(LIGHT_RETANGULO); // Inicialmente desativada, será controlada via teclado
    glDisable(LIGHT_RETANGULO);

    carregarTextura("images/Herobrine/Cabeca/cabeca-frente.png", skin1Texturas, 0); // Frente  
    carregarTextura("images/Herobrine/Cabeca/cabeca-tras.png", skin1Texturas, 1);   // Tras
    carregarTextura("images/Herobrine/Cabeca/cabeca-lado-e.png", skin1Texturas,2);  // Esquerda
    carregarTextura("images/Herobrine/Cabeca/cabeca-lado-d.png", skin1Texturas,3);  // Direita
    carregarTextura("images/Herobrine/Cabeca/cabeca-cima.png", skin1Texturas,4);    // Cima
    carregarTextura("images/Herobrine/Cabeca/cabeca-baixo.png", skin1Texturas, 5);  // Baixo

    carregarTextura("images/Herobrine/Tronco/tronco.png", skin1Texturas, 6);        // Frente  
    carregarTextura("images/Herobrine/Tronco/tronco-tras.png", skin1Texturas, 7);   // Tras
    carregarTextura("images/Herobrine/Tronco/tronco-lado.png", skin1Texturas, 8);   // Esquerdada
    carregarTextura("images/Herobrine/Tronco/tronco-lado.png", skin1Texturas, 9);   // Direita
    carregarTextura("images/Herobrine/Tronco/tronco-cima.png", skin1Texturas,10);   // Cima
    carregarTextura("images/Herobrine/Tronco/tronco-baixo.png",skin1Texturas, 11);  // Baixo

    carregarTextura("images/Herobrine/Braco/braco-frente.png",skin1Texturas, 12);   // Frente  
    carregarTextura("images/Herobrine/Braco/braco-tras.png",skin1Texturas, 13);     // Tras
    carregarTextura("images/Herobrine/Braco/braco-fora.png",skin1Texturas, 14);     // Esquerda
    carregarTextura("images/Herobrine/Braco/braco-dentro.png",skin1Texturas, 15);   // Direita
    carregarTextura("images/Herobrine/Braco/ombro.png",skin1Texturas, 16);          // Cima
    carregarTextura("images/Herobrine/Braco/mao.png",skin1Texturas, 17);            // Baixo

    carregarTextura("images/Herobrine/Perna/perna-frente.png",skin1Texturas, 18);   // Frente
    carregarTextura("images/Herobrine/Perna/perna-tras.png",skin1Texturas, 19);     // Tras
    carregarTextura("images/Herobrine/Perna/perna-e.png",skin1Texturas, 20);        // Esquerda
    carregarTextura("images/Herobrine/Perna/perna-d.png",skin1Texturas, 21);        // Direita
    carregarTextura("images/Herobrine/Perna/perna-cima.png",skin1Texturas, 22);     // Cima
    carregarTextura("images/Herobrine/Perna/perna-baixo.png",skin1Texturas, 23);    // Baixo  

    // Carregar todas as texturas para Skin 2
    carregarTextura("images/Monark/Cabeca/cabeca-frente.png", skin2Texturas, 0);    // Frente  
    carregarTextura("images/Monark/Cabeca/cabeca-tras.png", skin2Texturas, 1);      // Tras
    carregarTextura("images/Monark/Cabeca/cabeca-lado-e.png", skin2Texturas, 2);    // Esquerda
    carregarTextura("images/Monark/Cabeca/cabeca-lado-e.png", skin2Texturas,3);     // Direita
    carregarTextura("images/Monark/Cabeca/cabeca-cima.png", skin2Texturas,4);       // Cima
    carregarTextura("images/Monark/Cabeca/cabeca-baixo.png", skin2Texturas,5);      // Baixo

    carregarTextura("images/Monark/Tronco/tronco-frente.png", skin2Texturas,6); // Frente  
    carregarTextura("images/Monark/Tronco/tronco-frente.png", skin2Texturas,7); // Tras
    carregarTextura("images/Monark/Tronco/tronco-lado.png", skin2Texturas,8);   // Esquerda
    carregarTextura("images/Monark/Tronco/tronco-lado.png", skin2Texturas,9);   // Direita
    carregarTextura("images/Monark/Tronco/tronco-cima.png", skin2Texturas,10);  // Cima
    carregarTextura("images/Monark/Tronco/tronco-baixo.png", skin2Texturas,11); // Baixo

    carregarTextura("images/Monark/Braco/braco-frente.png", skin2Texturas,12);  // Frente  
    carregarTextura("images/Monark/Braco/braco-tras.png", skin2Texturas,13);    // Tras
    carregarTextura("images/Monark/Braco/braco-lado.png", skin2Texturas,14);    // Esquerda
    carregarTextura("images/Monark/Braco/braco-lado.png", skin2Texturas,15);    // Direita
    carregarTextura("images/Monark/Braco/braco-cima.png", skin2Texturas,16);    // Cima
    carregarTextura("images/Monark/Braco/braco-baixo.png", skin2Texturas,17);   // Baixo

    carregarTextura("images/Monark/Perna/perna-frente.png", skin2Texturas,18);
    carregarTextura("images/Monark/Perna/perna-cima.png", skin2Texturas,19);  
    carregarTextura("images/Monark/Perna/perna-lado.png", skin2Texturas,20);  
    carregarTextura("images/Monark/Perna/perna-lado.png", skin2Texturas,21); 
    carregarTextura("images/Monark/Perna/perna-cima.png", skin2Texturas,22);  
    carregarTextura("images/Monark/Perna/perna-cima.png", skin2Texturas,23); 

    // Carregar todas as texturas para Skin 3
    carregarTextura("images/Venom/Cabeca/cabeca-frente.png", skin3Texturas, 0);   // Textura 1 - Grama
    carregarTextura("images/Venom/Cabeca/cabeca-cima.png", skin3Texturas,1);   // Textura 1 - Grama
    carregarTextura("images/Venom/Cabeca/cabeca-lado.png", skin3Texturas,2);   // Textura 2 - Metal
    carregarTextura("images/Venom/Cabeca/cabeca-lado.png", skin3Texturas,3); // Textura 3 - Esponja
    carregarTextura("images/Venom/Cabeca/cabeca-cima.png", skin3Texturas,4);     // Textura 4 - Céu
    carregarTextura("images/Venom/Cabeca/cabeca-cima.png", skin3Texturas,5);   // Textura 5 - Metal]

    carregarTextura("images/Venom/Tronco/tronco-frente.png", skin3Texturas,6);   // Frente
    carregarTextura("images/Venom/Tronco/tronco-frente.png", skin3Texturas,7);   // Tras
    carregarTextura("images/Venom/Tronco/tronco-lado.png", skin3Texturas,8);   // Esquerda
    carregarTextura("images/Venom/Tronco/tronco-lado.png", skin3Texturas,9); // Direita
    carregarTextura("images/Venom/Tronco/tronco-cima.png", skin3Texturas,10);     // Cima
    carregarTextura("images/Venom/Tronco/tronco-baixo.png", skin3Texturas,11); // Baixo

    carregarTextura("images/Venom/Braco/braco-frente.png", skin3Texturas,12);   // Frente
    carregarTextura("images/Venom/Braco/braco-frente.png", skin3Texturas,13);   // Tras
    carregarTextura("images/Venom/Braco/braco-lado.png", skin3Texturas,14);   // Esquerda
    carregarTextura("images/Venom/Braco/braco-lado.png", skin3Texturas,15); // Direita
    carregarTextura("images/Venom/Braco/braco-cima.png", skin3Texturas,16);     // Cima
    carregarTextura("images/Venom/Braco/braco-baixo.png", skin3Texturas,17); // Baixo

    carregarTextura("images/Venom/Perna/perna-frente.png", skin3Texturas,18);   // Frente
    carregarTextura("images/Venom/Perna/perna-frente.png", skin3Texturas,19);   // Tras
    carregarTextura("images/Venom/Perna/perna-lado.png", skin3Texturas,20);   // Esquerda
    carregarTextura("images/Venom/Perna/perna-lado.png", skin3Texturas,21); // Direita
    carregarTextura("images/Venom/Perna/perna-cima.png", skin3Texturas,22);     // Cima
    carregarTextura("images/Venom/Perna/perna-cima.png", skin3Texturas,23); // Baixo

    // Define a skin inicial
    currentSkin = skin1Texturas;

    carregarTextura("images/Outros/grama.png", idsTextura, 24);
    carregarTextura("images/Outros/SOL.png", idsTextura ,25);

    carregarTextura("images/Outros/tocha-lado.png", idsTextura, 26); 
    carregarTextura("images/Outros/tocha-cima.png", idsTextura, 27); 
    carregarTextura("images/Outros/tocha-baixo.png", idsTextura, 28); 
    carregarTextura("images/Outros/tocha-frente-tras.png", idsTextura, 29); 


    carregarTextura("images/Outros/craftTable-frente.png", idsTextura, 30); 
    carregarTextura("images/Outros/craftTable-lado.png", idsTextura, 31); 
    carregarTextura("images/Outros/craftTable-cima.png", idsTextura, 32); 

    carregarTextura("images/Outros/fornalha-frente.png", idsTextura, 33); 
    carregarTextura("images/Outros/fornalha-lado.png", idsTextura, 34); 
    carregarTextura("images/Outros/fornalha-cima.png", idsTextura, 35); 
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)w / (double)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Personagem Minecraft");

    inicializa();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(teclado);
    glutKeyboardUpFunc(tecladoUp);
    glutSpecialFunc(tecladoEspecial); // Para pressionamento de teclas especiais
    glutMouseFunc(mouse);
    glutMotionFunc(arrastaMouse);
    glutTimerFunc(16, atualizaAnimacao, 0);

    int timeOfDaySubMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Dia", MENU_DIA);
    glutAddMenuEntry("Entardecer", MENU_ENTARDECER);
    glutAddMenuEntry("Noite", MENU_NOITE);

    // Criação do submenu "Textura Personagem"
    int textureSubMenu = glutCreateMenu(menu);
    glutAddMenuEntry("Herobrine", TEXTURE_SKIN_1);
    glutAddMenuEntry("Zoro", TEXTURE_SKIN_2);
    glutAddMenuEntry("Venom", TEXTURE_SKIN_3);
    // Adicione mais entradas conforme necessário

    // Criação do menu
    glutCreateMenu(menu); // Define a função de callback para o menu
    glutAddSubMenu("Hora/Clima", timeOfDaySubMenu);
    glutAddSubMenu("Skin", textureSubMenu); // Adiciona o submenu de texturas
    glutAddMenuEntry("Reset", MENU_RESET); // Adiciona a opção "Reset" ao menu
    glutAttachMenu(GLUT_RIGHT_BUTTON); // Anexa o menu ao botão direito do mouse

    glutMainLoop();
    return 0;
}