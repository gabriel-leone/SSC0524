import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

// Nome: Enzo Vasconcellos Pagotto, NºUSP: 13632844
// Nome: Gabriel Pietro Leone, NºUSP: 13874729

public class TesteCal {
    private Cal myCal;  // Declare myCal here

    @BeforeEach
    public void setUp() {
        myCal = new Cal();  // Initialize myCal here
    }

    @Test
    public void testCalAbril2024() {
        var abrilCal = myCal.cal(1, 30);
        assertEquals("1  2  3  4  5  6\n" +
                " 7  8  9 10 11 12 13\n" +
                "14 15 16 17 18 19 20\n" +
                "21 22 23 24 25 26 27\n" +
                "28 29 30".trim(), abrilCal.trim());
    }

    @Test
    public void testCalJaneiro2024() {
        var janeiroCal = myCal.cal(1, 31);
        assertEquals("1  2  3  4  5  6\n" +
                " 7  8  9 10 11 12 13\n" +
                "14 15 16 17 18 19 20\n" +
                "21 22 23 24 25 26 27\n" +
                "28 29 30 31\n".trim(), janeiroCal.trim());
    }

    @Test
    public void testCalMaio2009() {
        var maioCal = myCal.cal(5, 31);
        assertEquals("1  2\n" +
                " 3  4  5  6  7  8  9\n" +
                "10 11 12 13 14 15 16\n" +
                "17 18 19 20 21 22 23\n" +
                "24 25 26 27 28 29 30\n" +
                "31".trim(), maioCal.trim());
    }

    @Test
    public void testCalFevereiro() {
        var fevereiroCal = myCal.cal(2, 28);
        assertEquals("1  2  3  4  5\n"+
                " 6  7  8  9 10 11 12\n"+
                "13 14 15 16 17 18 19\n"+
                "20 21 22 23 24 25 26\n"+
                "27 28".trim(), fevereiroCal.trim());
    }

    @Test
    public void testCal19Dias() {
        var cal19Dias = myCal.cal(1, 19);
        assertEquals("1  2 14 15 16\n"+
                "17 18 19 20 21 22 23\n"+
                "24 25 26 27 28 29 30".trim(), cal19Dias.trim());
    }
}
