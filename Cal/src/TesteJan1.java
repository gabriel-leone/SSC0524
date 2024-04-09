import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

public class TesteJan1 {

    private Cal myCal;  // Declare myCal here

    @BeforeEach
    public void setUp() {
        myCal = new Cal();  // Initialize myCal here
    }

    @Test
    public void testJan2024() {
        assertEquals(1, myCal.jan1(2024));
    }

    @Test
    public void testJan2023() {
        assertEquals(0, myCal.jan1(2023));
    }

    @Test
    public void testJan2000() {
        assertEquals(6, myCal.jan1(2000));
    }

    @Test
    public void testJan1900() {
        assertEquals(1, myCal.jan1(1900));
    }

    @Test
    public void testJan2004() {
        assertEquals(4, myCal.jan1(2004));
    }
}
