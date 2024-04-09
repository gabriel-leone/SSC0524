import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

public class TesteIsLeap {

    private Cal myCal;  // Declare myCal here

    @BeforeEach
    public void setUp() {
        myCal = new Cal();  // Initialize myCal here
    }

    @Test
    public void testIsLeap2024() {
        assertTrue(myCal.isLeap(2024));
    }

    @Test
    public void testIsLeap2023() {
        assertFalse(myCal.isLeap(2023));
    }

    @Test
    public void testIsLeap2000() {
        assertTrue(myCal.isLeap(2000));
    }

    @Test
    public void testIsLeap1900() {
        assertFalse(myCal.isLeap(1900));
    }

    @Test
    public void testIsLeap2004() {
        assertTrue(myCal.isLeap(2004));
    }

    @Test
    public void testIsLeap1752() {
        assertTrue(myCal.isLeap(1752));
    }
}
