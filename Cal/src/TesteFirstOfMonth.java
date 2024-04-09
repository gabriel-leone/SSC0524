import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

public class TesteFirstOfMonth {

    private Cal myCal;  // Declare myCal here

    @BeforeEach
    public void setUp() {
        myCal = new Cal();  // Initialize myCal here
    }

    @Test
    public void testFirstOfMonthAbril2024() {
        assertEquals(1, myCal.firstOfMonth(4, 2024));
    }

    @Test
    public void testFirstOfMonthJaneiro2024() {
        assertEquals(1, myCal.firstOfMonth(1, 2024));
    }

    @Test
    public void testFirstOfMonthMaio2009() {
        assertEquals(5, myCal.firstOfMonth(5, 2009));
    }
}
