# trabapc
Trabalho de Algoritmos e Programação de Computadores feito por Gabriel Delolmo Erhardt em C.

# Objetivo
Criar um jogo onde é possível percorrer um labirinto encontrando e lutando contra vampiros controlados pela máquina. As lutas ocorrem em turnos onde cada jogador pode escolher dentre cinco ações, ataque rápido, ataque forte, defesa, usar poção(se ele possuir alguma) e fugir.

Durante a exploração pode-se encontrar armas, armuduras e poções para ajudar no combate!

O jogo termina ao encontrar e derrotar o vampiro chefe, Drácula. Ao final pode-se ver algumas capturas do jogo.

# Como jogar

O jogo foi criado para o Linux Mint e compilado com o GCC, compatibilidade com outros sistemas operacionais não é garantida. O primeiro passo é baixar os arquivos trab3apc.c.
Caso o jogador deseje usar o mapa providenciado pelos monitores da disciplina também é possível baixar o arquivo mapa.txt no mesmo local onde se encontra trab3apc.c, mas mapa.txt não é necessário para o funcionamento do jogo.
Então, deve-se executar o código a seguir no terminal na pasta onde o arquivo foi baixado.

```
gcc -ansi trab3apc.c -o jogo -lm
```

É recomendado maximizar o terminal antes de iniciar o jogo uma vez que o espaço da tela que o jogo ocupa tende a ser maior do que o dísponivel no tamanho padrão da maioria dos terminais (o tamanho pode ser configurado posteriormente).
Então para jogar basta rodar com:

```
./jogo
```

# Controles

Use as setas verticais para escolher a opção nos menus, enquanto a seta para a esquerda e a tecla Z confirmam a seleção. A tecla X permite abrir o menu do jogo enquanto explora o mapa. Nos menus a tecla para a direita funciona como a tecla Z, essa função pode ser desabilitada nas opções. Use as setas para se mover pelo mapa e Z para pegar itens do chão.

# Combate

Num combate é possível escolher entre um ataque fraco, um ataque forte, se defender, usar uma poção ou fugir.
O ataque fraco dá o seu ataque em dano no adversário, e rouba um pouco da vida causada no ataque para você.
O ataque forte dá o dobro do seu ataque em dano, mas tem uma chance de te deixar atordoado por um turno e não rouba vida.
Ao se defender você absorve 50% do dano, se os dois oponentes se defenderem ao mesmo tempo, ambos recuperam 10% da vida máxima. Além disso, se o oponente usar um ataque fraco, há uma chance de ser atordoado proporcional a precisão dele. Mas se usar um ataque forte será atordoado com certeza.
Ao usar uma poção, você recupera vida de acordo com a força da poção, mas se torna vulnerável, sendo garantido o acerto do seu oponente.
Por fim, toda vez que fugir de uma batalha o seu oponente subirá de nível ficando mais forte. Cuidado!

A medida que ganha batalhas, o seu nivel sobe. A cada nivel você recebe três pontos para distribuir entre ataque, vida máxima, precisão e roubo de vida. Para distribuir os pontos basta entrar no menu enquanto joga.

![Jogador andando pelo mapa](https://github.com/delanoerhardt/Trab-Apc/blob/master/foto1.png)
Jogador andando pelo mapa
![Batalha contra um vampiro](https://github.com/delanoerhardt/Trab-Apc/blob/master/foto2.png)
Batalha contra um vampiro
![Localizando e vendo os detalhes do Drácula](https://github.com/delanoerhardt/Trab-Apc/blob/master/foto3.png)
Localizando e vendo os detalhes do Drácula
