import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.AfterEach;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

// Nome: Enzo Vasconcellos Pagotto, NºUSP: 13632844
// Nome: Gabriel Pietro Leone, NºUSP: 13874729

public class TesteMain {
    private final ByteArrayOutputStream outContent = new ByteArrayOutputStream();
    private final PrintStream originalOut = System.out;
    private final ByteArrayOutputStream errContent = new ByteArrayOutputStream();
    private final PrintStream originalErr = System.err;

    @BeforeEach
    public void setUpStreams() {
        System.setOut(new PrintStream(outContent));
        System.setErr(new PrintStream(errContent)); // redireciona o fluxo de erro padrão
    }

    @AfterEach
    public void restoreStreams() {
        System.setOut(originalOut);
        System.setErr(originalErr); // restaura o fluxo de erro padrão
    }

    @Test
    public void testWithoutParameters() {
        String[] argv = {};
        Cal.main(argv);
        assertEquals("Maio 2024\n" +
                "Do Se Te Qa Qi Se Sa\n" +
                "          1  2  3  4\n" +
                " 5  6  7  8  9 10 11\n" +
                "12 13 14 15 16 17 18\n" +
                "19 20 21 22 23 24 25\n" +
                "26 27 28 29 30 31 \n".trim(), outContent.toString().trim());
    }

    @Test
    public void testWithParametersJan2024() {
        String[] argv = {"1", "2024"};
        Cal.main(argv);
        assertEquals("Janeiro 2024\n" +
                "Do Se Te Qa Qi Se Sa\n" +
                "    1  2  3  4  5  6\n" +
                " 7  8  9 10 11 12 13\n" +
                "14 15 16 17 18 19 20\n" +
                "21 22 23 24 25 26 27\n" +
                "28 29 30 31 \n".trim(), outContent.toString().trim());
    }

    @Test
    public void testWithParametersMay2009() {
        String[] argv = {"5", "2009"};
        Cal.main(argv);
        assertEquals("Maio 2009\n" +
                "Do Se Te Qa Qi Se Sa\n" +
                "                1  2\n" +
                " 3  4  5  6  7  8  9\n" +
                "10 11 12 13 14 15 16\n" +
                "17 18 19 20 21 22 23\n" +
                "24 25 26 27 28 29 30\n" +
                "31 \n".trim(), outContent.toString().trim());
    }

    @Test
    public void testWithParameter2024() {
        String[] argv = {"2024"};
        Cal.main(argv);
        var months = new String[] {
                "Janeiro 2024", "Fevereiro 2024", "Março 2024", "Abril 2024",
                "Maio 2024", "Junho 2024", "Julho 2024", "Agosto 2024",
                "Setembro 2024", "Outubro 2024", "Novembro 2024", "Dezembro 2024"
        };
        for (String month : months) {
            assertTrue(outContent.toString().contains(month));
        }
    }

    @Test
    public void testWithSingleParameterInvalidYear0() {
        String[] argv = {"0"};
        Cal.main(argv);
        assertEquals("Cal: 0: ano invalido.".trim(), errContent.toString().trim());
    }

    @Test
    public void testWithSingleParameterInvalidYear10000() {
        String[] argv = {"10000"};
        Cal.main(argv);
        assertEquals("Cal: 10000: ano invalido.".trim(), errContent.toString().trim());
    }

    @Test
    public void testWithInvalidMonth13() {
        String[] argv = {"13", "2024"};
        Cal.main(argv);
        assertEquals("Cal: 13: mes invalido.".trim(), errContent.toString().trim());
    }

    @Test
    public void testWithInvalidMonth0() {
        String[] argv = {"0", "2024"};
        Cal.main(argv);
        assertEquals("Cal: 0: mes invalido.".trim(), errContent.toString().trim());
    }

    @Test
    public void testWithInvalidYear10000() {
        String[] argv = {"1", "10000"};
        Cal.main(argv);
        assertEquals("Cal: 10000: ano invalido.".trim(), errContent.toString().trim());
    }

    @Test
    public void testWithInvalidYear0() {
        String[] argv = {"1", "0"};
        Cal.main(argv);
        assertEquals("Cal: 0: ano invalido.".trim(), errContent.toString().trim());
    }

    @Test
    public void testWithInvalidYearNegative() {
        String[] argv = {"1", "-1"};
        Cal.main(argv);
        assertEquals("Cal: -1: ano invalido.".trim(), errContent.toString().trim());
    }

    @Test
    public void testWithInvalidYearNegativeMonth() {
        String[] argv = {"-1", "2024"};
        Cal.main(argv);
        assertEquals("Cal: -1: mes invalido.".trim(), errContent.toString().trim());
    }

    @Test
    public void testWithAlphabeticMonth() {
        String[] argv = {"a", "2024"};
        Cal.main(argv);
        assertEquals("Cal: a: mes invalido.".trim(), errContent.toString().trim());
    }

    @Test
    public void testWithAlphabeticYear() {
        String[] argv = {"1", "a"};
        Cal.main(argv);
        assertEquals("Cal: a: ano invalido.".trim(), errContent.toString().trim());
    }

    @Test
    public void testWithAlphabeticMonthAndYear() {
        String[] argv = {"a", "a"};
        Cal.main(argv);
        assertEquals("Cal: a: mes invalido.".trim(), errContent.toString().trim());
    }

    @Test
    public void testWithSingleArgumentAlphabeticYear() {
        String[] argv = {"a"};
        Cal.main(argv);
        assertEquals("Cal: a: ano invalido.".trim(), errContent.toString().trim());
    }
}