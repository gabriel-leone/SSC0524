package org.gabrielleone;

public class Boliche {
    public int computaPlacar(String jogadas) throws IllegalArgumentException {

        VerificaInput(jogadas);

        int pontuacao = 0;
        for (int i = 0; i < 20; i++) {
            if (jogadas.charAt(i) == 'X') {
                pontuacao += CalculaStrike(jogadas, i);
            } else if (jogadas.charAt(i) == '/') {
                pontuacao += CalculaSpare(jogadas, i);
            } else {
                pontuacao += AlteraPontuacao(String.valueOf(jogadas.charAt(i)));
            }
        }
        return pontuacao;
    }

    private void VerificaInput(String jogadas) {
        if (jogadas.length() != 21) {
            throw new IllegalArgumentException("Quantidade de jogadas inválida");
        }

        for (int i = 0; i < jogadas.length(); i++) {
            if (jogadas.charAt(i) != 'X' && jogadas.charAt(i) != '/' && jogadas.charAt(i) != '-'
                    && !Character.isDigit(jogadas.charAt(i))) {
                throw new IllegalArgumentException("Caracter inválido");
            }
        }
    }

    private int CalculaStrike(String jogadas, int jogadaAtual) {
        int pontuacaoStrike = 0;
        pontuacaoStrike += AlteraPontuacao("X");
        if (jogadas.charAt(jogadaAtual + 2) == 'X') {
            pontuacaoStrike += CalculaStrike(jogadas, jogadaAtual + 2);
            return pontuacaoStrike;
        } else {
            pontuacaoStrike += AlteraPontuacao(String.valueOf(jogadas.charAt(jogadaAtual + 2)));
            return pontuacaoStrike;
        }
    }

    private int CalculaSpare(String jogadas, int jogadaAtual) {
        int pontuacaoSpare = 0;
        pontuacaoSpare += AlteraPontuacao("/");
        pontuacaoSpare -= AlteraPontuacao(String.valueOf(jogadas.charAt(jogadaAtual - 1)));
        pontuacaoSpare += AlteraPontuacao(String.valueOf(jogadas.charAt(jogadaAtual + 1)));
        return pontuacaoSpare;
    }

    private int AlteraPontuacao(String jogada) {
        return switch (jogada) {
            case "X", "/" -> 10;
            case "-" -> 0;
            default -> Integer.parseInt(jogada);
        };
    }
}
