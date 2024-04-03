package org.gabrielleone;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.assertEquals;

public class TestBoliche2 {

    @Test
    public void testComputaPlacar() {
        Boliche boliche = new Boliche();
        // Teste 1
        String jogadas = "8070539/9/X-80513/90-";
        int pontuacao = boliche.computaPlacar(jogadas);
        assertEquals(122, pontuacao);

        // Teste 2
        String jogadas2 = "8/90447290X-X-80359/7";
        int pontuacao2 = boliche.computaPlacar(jogadas2);
        assertEquals(133, pontuacao2);

        // Teste 3
        String jogada3 = "123456789abc123456789";
        Assertions.assertThrows(IllegalArgumentException.class, () -> {
            boliche.computaPlacar(jogada3);
        });

        // Teste 4
        String jogada4 = "123456789123456789123456789";
        Assertions.assertThrows(IllegalArgumentException.class, () -> {
            boliche.computaPlacar(jogada4);
        });

        // Teste 5
        String jogada5 = "123";
        Assertions.assertThrows(IllegalArgumentException.class, () -> {
            boliche.computaPlacar(jogada5);
        });

    }

}
