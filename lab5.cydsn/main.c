#include "project.h" 

/* Forward declarations */
static void FourDigit74HC595_sendData(uint8_t data);
static void FourDigit74HC595_sendOneDigit(uint8_t position, uint8_t digit, uint8_t dot);
static void initMatrix(void);
static void readMatrix(void);
static void checkPassword(void);
static void showSuccessAnimation(void);
static void showFailAnimation(void);
static void cyclicDisplay(void);

/* Segment codes */
static uint8_t LED_NUM[] = { 
    0xC0,   //0 
    0xF9,   //1 
    0xA4,   //2 
    0xB0,   //3 
    0x99,   //4 
    0x92,   //5 
    0x82,   //6 
    0xF8,   //7 
    0x80,   //8 
    0x90,   //9 
    0xBF,   //- (dash)
    0x88,   //A 
    0x83,   //b 
    0xC6,   //C 
    0xA1,   //d 
    0x86,   //E 
    0x8E,   //F
    0xFF,   //OFF (blank)
}; 

/* Global variables */
static uint8_t led_counter = 0;
static uint8_t display_buffer[8] = {7, 6, 5, 4, 3, 2, 1, 0};

/* Password system variables */
#define PASSWORD_LENGTH 4
#define CORRECT_PASSWORD {1, 2, 3, 4}  // Задайте свій пароль тут
static uint8_t entered_password[PASSWORD_LENGTH];
static uint8_t password_index = 0;
static uint8_t password_mode = 0;  // 0 - normal, 1 - entering password, 2 - checking

/* Cyclic display variables */
static uint8_t cyclic_pattern[] = {8, 11, 12, 13, 14, 15, 10, 17};  // 8AbCdEF-_
static uint8_t cyclic_index = 0;
static uint16_t cyclic_counter = 0;

/* Timer interrupt for dynamic display */
CY_ISR(Timer_Int_Handler2)
{
    FourDigit74HC595_sendOneDigit(led_counter, display_buffer[led_counter], 0);
    led_counter++;
    if(led_counter > 7) led_counter = 0;
    
    // Cyclic display update
    cyclic_counter++;
    if(cyclic_counter >= 500) {  // Adjust speed here
        cyclic_counter = 0;
        if(password_mode == 0) {
            cyclicDisplay();
        }
    }
}

/* Send data to shift register */     
static void FourDigit74HC595_sendData(uint8_t data) {     
    for(uint8_t i = 0; i < 8; i++) {         
        if(data & (0x80 >> i)) { 
            Pin_DO_Write(1); 
        } else { 
            Pin_DO_Write(0); 
        } 
        Pin_CLK_Write(1); 
        Pin_CLK_Write(0); 
    } 
} 

/* Send one digit to display */ 
static void FourDigit74HC595_sendOneDigit(uint8_t position, uint8_t digit, uint8_t dot)  
{ 
    if(position >= 8) {
        FourDigit74HC595_sendData(0xFF); 
        FourDigit74HC595_sendData(0xFF); 
        return;
    } 
    FourDigit74HC595_sendData(0xFF & ~(1 << position));     
    if(dot) { 
        FourDigit74HC595_sendData(LED_NUM[digit] & 0x7F); 
    } else { 
        FourDigit74HC595_sendData(LED_NUM[digit]); 
    } 
    Pin_Latch_Write(1);
    Pin_Latch_Write(0); 
} 

/* Cyclic display function - scroll pattern left to right */
static void cyclicDisplay(void) {
    // Shift all positions left
    for(int i = 0; i < 7; i++) {
        display_buffer[i] = display_buffer[i + 1];
    }
    // Add new symbol at the right
    display_buffer[7] = cyclic_pattern[cyclic_index];
    cyclic_index++;
    if(cyclic_index >= sizeof(cyclic_pattern)) {
        cyclic_index = 0;
    }
}
 
/* Keypad matrix [ROW][COLUMN] */ 
static uint8_t keys[4][3] = { 
    {1, 2, 3}, 
    {4, 5, 6}, 
    {7, 8, 9}, 
    {10, 0, 11},  // *, 0, #
}; 

/* Arrays of function pointers */ 
static void (*COLUMN_x_SetDriveMode[3])(uint8_t mode) = { 
    COLUMN_0_SetDriveMode, 
    COLUMN_1_SetDriveMode, 
    COLUMN_2_SetDriveMode 
};   

static void (*COLUMN_x_Write[3])(uint8_t value) = { 
    COLUMN_0_Write, 
    COLUMN_1_Write, 
    COLUMN_2_Write 
}; 

static uint8 (*ROW_x_Read[4])() = { 
    ROW_0_Read, 
    ROW_1_Read, 
    ROW_2_Read, 
    ROW_3_Read 
}; 

/* Initialize keypad matrix */ 
static void initMatrix(void) 
{     
    for(int column_index = 0; column_index < 3; column_index++) { 
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_DIG_HIZ); 
    } 
} 

/* Read keypad matrix */ 
static void readMatrix(void) 
{ 
    uint8_t row_counter = sizeof(ROW_x_Read)/sizeof(ROW_x_Read[0]); 
    uint8_t column_counter = sizeof(COLUMN_x_Write)/sizeof(COLUMN_x_Write[0]);     
    
    for(int column_index = 0; column_index < column_counter; column_index++) { 
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_STRONG); 
        COLUMN_x_Write[column_index](0);         
        
        for(int row_index = 0; row_index < row_counter; row_index++) { 
            keys[row_index][column_index] = ROW_x_Read[row_index](); 
        } 
        
        COLUMN_x_SetDriveMode[column_index](COLUMN_0_DM_DIG_HIZ); 
    } 
} 

/* Check if entered password is correct */
static void checkPassword(void) {
    uint8_t correct_password[PASSWORD_LENGTH] = CORRECT_PASSWORD;
    uint8_t is_correct = 1;
    
    for(int i = 0; i < PASSWORD_LENGTH; i++) {
        if(entered_password[i] != correct_password[i]) {
            is_correct = 0;
            break;
        }
    }
    
    if(is_correct) {
        showSuccessAnimation();
    } else {
        showFailAnimation();
    }
    
    // Reset password mode
    password_mode = 0;
    password_index = 0;
    cyclic_index = 0;
}

/* Success animation - running light */
static void showSuccessAnimation(void) {
    // Fill with blank
    for(int i = 0; i < 8; i++) {
        display_buffer[i] = 17;  // blank
    }
    
    // Running light effect (3 cycles)
    for(int cycle = 0; cycle < 3; cycle++) {
        for(int pos = 0; pos < 8; pos++) {
            display_buffer[pos] = 8;  // Show '8'
            CyDelay(100);
            display_buffer[pos] = 17;  // blank
        }
    }
    
    // Show "PASS" pattern
    display_buffer[0] = 17;  // blank
    display_buffer[1] = 17;  // blank
    display_buffer[2] = 0;   // P (use 0)
    display_buffer[3] = 11;  // A
    display_buffer[4] = 5;   // S (use 5)
    display_buffer[5] = 5;   // S (use 5)
    display_buffer[6] = 17;  // blank
    display_buffer[7] = 17;  // blank
    CyDelay(2000);
}

/* Fail animation - blinking dashes */
static void showFailAnimation(void) {
    // Blink error pattern 5 times
    for(int i = 0; i < 5; i++) {
        // Show all dashes
        for(int j = 0; j < 8; j++) {
            display_buffer[j] = 10;  // dash
        }
        CyDelay(200);
        
        // All blank
        for(int j = 0; j < 8; j++) {
            display_buffer[j] = 17;  // blank
        }
        CyDelay(200);
    }
    
    // Show "FAIL" pattern
    display_buffer[0] = 17;  // blank
    display_buffer[1] = 15;  // F
    display_buffer[2] = 11;  // A
    display_buffer[3] = 1;   // I (use 1)
    display_buffer[4] = 6;   // L (use 6)
    display_buffer[5] = 17;  // blank
    display_buffer[6] = 17;  // blank
    display_buffer[7] = 17;  // blank
    CyDelay(2000);
}

int main(void) 
{ 
    CyGlobalIntEnable;
    
    Timer_Start();
    Timer_Int_StartEx(Timer_Int_Handler2);
    
    initMatrix(); 
    uint8_t last_state = 12;
    
    // Initialize display with cyclic pattern
    for(int i = 0; i < 8; i++) {
        display_buffer[i] = cyclic_pattern[i % sizeof(cyclic_pattern)];
    }
    
    for(;;) 
    { 
        readMatrix(); 
        
        // Check for '#' key to start password entry
        if(keys[3][2] == 0 && last_state != 11) {
            last_state = 11;
            if(password_mode == 0) {
                // Start password entry mode
                password_mode = 1;
                password_index = 0;
                // Clear display
                for(int i = 0; i < 8; i++) {
                    display_buffer[i] = 17;  // blank
                }
                // Show "----" for password entry
                for(int i = 0; i < PASSWORD_LENGTH; i++) {
                    display_buffer[i] = 10;  // dash
                }
            } else if(password_mode == 1) {
                // Submit password
                password_mode = 2;
                checkPassword();
            }
        }
        if(keys[3][2] == 1 && last_state == 11) {
            last_state = 12;
        }
        
        // Handle digit input during password mode
        if(password_mode == 1) {
            // Check digits 0-9
            for(int row = 0; row < 4; row++) {
                for(int col = 0; col < 3; col++) {
                    uint8_t key_value = keys[row][col];
                    uint8_t button_number;
                    
                    if(row == 0 && col == 0) button_number = 1;
                    else if(row == 0 && col == 1) button_number = 2;
                    else if(row == 0 && col == 2) button_number = 3;
                    else if(row == 1 && col == 0) button_number = 4;
                    else if(row == 1 && col == 1) button_number = 5;
                    else if(row == 1 && col == 2) button_number = 6;
                    else if(row == 2 && col == 0) button_number = 7;
                    else if(row == 2 && col == 1) button_number = 8;
                    else if(row == 2 && col == 2) button_number = 9;
                    else if(row == 3 && col == 1) button_number = 0;
                    else continue;  // Skip * and #
                    
                    if(key_value == 0 && last_state != button_number) {
                        last_state = button_number;
                        
                        if(password_index < PASSWORD_LENGTH) {
                            entered_password[password_index] = button_number;
                            display_buffer[password_index] = button_number;
                            password_index++;
                        }
                    }
                    if(key_value == 1 && last_state == button_number) {
                        last_state = 12;
                    }
                }
            }
        }
        
        CyDelay(10);  // Small delay for debouncing
    } 
}