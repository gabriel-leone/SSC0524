import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

// Nome: Enzo Vasconcellos Pagotto, NºUSP: 13632844
// Nome: Gabriel Pietro Leone, NºUSP: 13874729

public class TesteNumberOfDays {

    private Cal myCal;  // Declare myCal here

    @BeforeEach
    public void setUp() {
        myCal = new Cal();  // Initialize myCal here
    }

    @Test
    public void testNumberOfDaysAbril2024() {
        assertEquals(30, myCal.numberOfDays(4, 2024));
    }

    @Test
    public void testNumberOfDaysJaneiro2024() {
        assertEquals(31, myCal.numberOfDays(1, 2024));
    }

    @Test
    public void testNumberOfDaysFevereiro2024() {
        assertEquals(29, myCal.numberOfDays(2, 2024));
    }

    @Test
    public void testNumberOfDaysFevereiro2023() { assertEquals(28, myCal.numberOfDays(2, 2023)); }

    @Test
    public void testNumberOfDaysSetembro1752() { assertEquals(19, myCal.numberOfDays(9, 1752)); }

    @Test void testNumberOfDaysOutubro1752() { assertEquals(31, myCal.numberOfDays(10, 1752)); }
}
