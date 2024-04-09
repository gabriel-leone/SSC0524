import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.AfterEach;

import static org.junit.jupiter.api.Assertions.assertEquals;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

public class TesteMain {
    private final ByteArrayOutputStream outContent = new ByteArrayOutputStream();
    private final PrintStream originalOut = System.out;

    @BeforeEach
    public void setUpStreams() {
        System.setOut(new PrintStream(outContent));
    }

    @AfterEach
    public void restoreStreams() {
        System.setOut(originalOut);
    }

    @Test
    public void testWithoutParameters() {
        String[] argv = {};
        Cal.main(argv);
        assertEquals("Abril 2024\n" +
                "Do Se Te Qa Qi Se Sa\n" +
                "    1  2  3  4  5  6\n" +
                " 7  8  9 10 11 12 13\n" +
                "14 15 16 17 18 19 20\n" +
                "21 22 23 24 25 26 27\n" +
                "28 29 30 \n".trim(), outContent.toString().trim());
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
}