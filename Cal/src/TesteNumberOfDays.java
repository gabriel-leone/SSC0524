import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

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
    public void testNumberOfDaysMaio2009() {
        assertEquals(31, myCal.numberOfDays(5, 2009));
    }

    @Test
    public void testNumberOfDaysFevereiro2024() {
        assertEquals(29, myCal.numberOfDays(2, 2024));
    }

    @Test
    public void testNumberOfDaysFevereiro2023() {
        assertEquals(28, myCal.numberOfDays(2, 2023));
    }
}
