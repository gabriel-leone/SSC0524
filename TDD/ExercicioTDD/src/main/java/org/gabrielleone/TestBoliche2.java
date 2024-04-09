package org.gabrielleone;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.assertEquals;

public class TestBoliche2 {

    @Test
    public void testComputaPlacar() {
        Boliche boliche = new Boliche();

        // Teste 1
        String jogadas = "53535353535353535353-";
        int pontuacao = boliche.computaPlacar(jogadas);
        assertEquals(80, pontuacao);

        // Teste 2
        String jogadas2 = "8070539090X-80513290-";
        int pontuacao2 = boliche.computaPlacar(jogadas2);
        assertEquals(87, pontuacao2);

        // Teste 3
        String jogadas3 = "8090447290X-X-803590-";
        int pontuacao3 = boliche.computaPlacar(jogadas3);
        assertEquals(114, pontuacao3);

        // Teste 4
        String jogadas4 = "8/90447290X-X-80359/7";
        int pontuacao4 = boliche.computaPlacar(jogadas4);
        assertEquals(133, pontuacao4);

        // Teste 5
        String jogada4 = "123456789123456789123456789";
        Assertions.assertThrows(IllegalArgumentException.class, () -> {
            boliche.computaPlacar(jogada4);
        });

        // Teste 6
        String jogada5 = "123";
        Assertions.assertThrows(IllegalArgumentException.class, () -> {
            boliche.computaPlacar(jogada5);
        });
    }
}
