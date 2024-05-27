import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

// Nome: Enzo Vasconcellos Pagotto, NºUSP: 13632844
// Nome: Gabriel Pietro Leone, NºUSP: 13874729

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
        assertEquals(6, myCal.jan1(2000));
    }

    @Test
    public void testJan1790() { assertEquals(5, myCal.jan1(1790)); }

    @Test
    public void testJan1752() { assertEquals(3, myCal.jan1(1752)); }

    @Test
    public void testJan1700() { assertEquals(1, myCal.jan1(1700)); }

    @Test
    public void testJan1500() { assertEquals(3, myCal.jan1(1500)); }
}
