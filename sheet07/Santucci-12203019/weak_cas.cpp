bool weakCAS(word *a, word expected, word new_value) {
    word current_value = LL(a); // Perform Load-Linked

    if (current_value == expected) {
        // Try to perform Store-Conditional
        if (SC(a, new_value)) {
            // SC succeeded (memory location was not changed between LL and SC)
            return true;
        } else {
            // SC failed (memory location was changed between LL and SC)
            return false;
        }
    } else {
        // CAS failed because current_value != expected
        return false;
    }
}
