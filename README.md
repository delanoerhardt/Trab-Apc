# trabapc
Trabalho de Algoritmos e Programação de Computadores feito por Gabriel Delolmo Erhardt em C.

# Objetivo
Criar um jogo onde é possível percorrer um labirinto encontrando e lutando contra vampiros controlados pela máquina. As lutas ocorrem em turnos onde cada jogador pode escolher dentre cinco ações, ataque rápido, ataque forte, defesa, usar poção(se ele possuir alguma) e fugir.

Durante a exploração pode-se encontrar armas, armuduras e poções para ajudar no combate!

O jogo termina ao encontrar e derrotar o vampiro chefe, Drácula. Ao final pode-se ver algumas capturas do jogo.

# Como jogar

O jogo foi criado para o Linux Mint e compilado com o GCC, compatibilidade com outros sistemas operacionais não é garantida. É necessário compilar o código para jogar, o que pode ser feito baixando o arquivo trab3apc.c e executando o código a seguir no terminal na pasta onde o arquivo foi baixado.

```
gcc -ansi trab3apc.c -o jogo -lm
```

É recomendado maximizar o terminal antes de iniciar o jogo uma vez que o espaço da tela que o jogo ocupa tende a ser maior do que o dísponivel no tamanho padrão da maioria dos terminais (o tamanho pode ser configurado posteriormente).
Então para jogar basta rodar com

```
./jogo
```

