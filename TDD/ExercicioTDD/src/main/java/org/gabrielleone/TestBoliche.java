package org.gabrielleone;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class TestBoliche {

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
    }

}
