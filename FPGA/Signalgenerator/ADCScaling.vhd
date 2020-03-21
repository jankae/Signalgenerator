----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    14:29:42 03/21/2020 
-- Design Name: 
-- Module Name:    ADCScaling - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity ADCScaling is
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           ADC_I : in  STD_LOGIC_VECTOR (9 downto 0);
           ADC_Q : in  STD_LOGIC_VECTOR (9 downto 0);
           NEW_IN : in  STD_LOGIC;
           OFFSET_I : in  STD_LOGIC_VECTOR (9 downto 0);
           OFFSET_Q : in  STD_LOGIC_VECTOR (9 downto 0);
           MULT_I : in  STD_LOGIC_VECTOR (9 downto 0);
           MULT_Q : in  STD_LOGIC_VECTOR (9 downto 0);
           SHIFT_I : in  STD_LOGIC_VECTOR (3 downto 0);
           SHIFT_Q : in  STD_LOGIC_VECTOR (3 downto 0);
           OUT_I : out  STD_LOGIC_VECTOR (9 downto 0);
           OUT_Q : out  STD_LOGIC_VECTOR (9 downto 0);
           NEW_OUT : out  STD_LOGIC;
           CLIPPED_I : out  STD_LOGIC;
           CLIPPED_Q : out  STD_LOGIC);
end ADCScaling;

architecture Behavioral of ADCScaling is
COMPONENT ADCScalingMult
  PORT (
    clk : IN STD_LOGIC;
    a : IN STD_LOGIC_VECTOR(10 DOWNTO 0);
    b : IN STD_LOGIC_VECTOR(9 DOWNTO 0);
    ce : IN STD_LOGIC;
    sclr : IN STD_LOGIC;
    p : OUT STD_LOGIC_VECTOR(20 DOWNTO 0)
  );
END COMPONENT;
	signal mult_a : std_logic_vector(10 downto 0);
	signal mult_b : std_logic_vector(9 downto 0);
	signal mult_out : std_logic_vector(20 downto 0);
	signal adc_no_offset_Q : std_logic_vector(10 downto 0);
	signal adc_scaled_I : std_logic_vector(20 downto 0);
	signal adc_scaled_Q : std_logic_vector(20 downto 0);
	signal state : std_logic_vector(10 downto 0);
	signal adc_shifted1_I : std_logic_vector(20 downto 0);
	signal adc_shifted2_I : std_logic_vector(20 downto 0);
	signal adc_shifted4_I : std_logic_vector(20 downto 0);
	signal adc_shifted8_I : std_logic_vector(20 downto 0);
	signal adc_shifted1_Q : std_logic_vector(20 downto 0);
	signal adc_shifted2_Q : std_logic_vector(20 downto 0);
	signal adc_shifted4_Q : std_logic_vector(20 downto 0);
	signal adc_shifted8_Q : std_logic_vector(20 downto 0);
	signal adc_constrained_I : std_logic_vector(9 downto 0);
	signal adc_constrained_Q : std_logic_vector(9 downto 0);
begin
	Multiplier : ADCScalingMult
	PORT MAP (
		clk => CLK,
		a => mult_a,
		b => mult_b,
		ce => '1',
		sclr => RESET,
		p => mult_out
	);
  
	process(CLK, RESET)
	begin
		if(rising_edge(CLK)) then
			if(RESET = '1') then
				state <= (others => '0');
				adc_no_offset_Q <= (others => '0');
				adc_scaled_I <= (others => '0');
				adc_shifted1_I <= (others => '0');
				adc_shifted2_I <= (others => '0');
				adc_shifted4_I <= (others => '0');
				adc_shifted8_I <= (others => '0');
				adc_constrained_I <= (others => '0');
				adc_scaled_Q <= (others => '0');
				adc_shifted1_Q <= (others => '0');
				adc_shifted2_Q <= (others => '0');
				adc_shifted4_Q <= (others => '0');
				adc_shifted8_Q <= (others => '0');
				adc_constrained_Q <= (others => '0');
			else
				state <= state(9 downto 0) & NEW_IN;
				if(NEW_IN = '1') then
					mult_a <= std_logic_vector(to_signed(to_integer(signed(ADC_I)) - to_integer(signed(OFFSET_I)), 11));
					mult_b <= MULT_I;
					adc_no_offset_Q <= std_logic_vector(to_signed(to_integer(signed(ADC_Q)) - to_integer(signed(OFFSET_Q)), 11));
				elsif (state(0)='1') then
					-- first cycle after new sample, load multiplier with Q ADC
					mult_a <= adc_no_offset_Q;
					mult_b <= MULT_Q;
				elsif (state(2)='1') then
					-- third cycle, multiplication of I sample is ready
					adc_scaled_I <= mult_out;
				elsif (state(3)='1') then
					-- forth cycle, multiplication of Q sample is ready
					adc_scaled_Q <= mult_out;
				end if;
				if (state(9)='1') then
					-- barrel shifters and constrainig is done
					OUT_I <= adc_constrained_I;
					OUT_Q <= adc_constrained_Q;
					NEW_OUT <= '1';
				elsif (state(10)='1') then
					-- reset new out flag
					NEW_OUT <= '0';
				end if;
				-- barrel shifters
				if(SHIFT_I(3)='1') then
					adc_shifted8_I <= (20 downto 13 => adc_scaled_I(20)) & adc_scaled_I(20 downto 8);
				else
					adc_shifted8_I <= adc_scaled_I;
				end if;
				if(SHIFT_I(2)='1') then
					adc_shifted4_I <= (20 downto 17 => adc_shifted8_I(20)) & adc_shifted8_I(20 downto 4);
				else
					adc_shifted4_I <= adc_shifted8_I;
				end if;
				if(SHIFT_I(1)='1') then
					adc_shifted2_I <= (20 downto 19 => adc_shifted4_I(20)) & adc_shifted4_I(20 downto 2);
				else
					adc_shifted2_I <= adc_shifted4_I;
				end if;
				if(SHIFT_I(0)='1') then
					adc_shifted1_I <= adc_shifted2_I(20) & adc_shifted2_I(20 downto 1);
				else
					adc_shifted1_I <= adc_shifted2_I;
				end if;
				if(SHIFT_Q(3)='1') then
					adc_shifted8_Q <= (20 downto 13 => adc_scaled_Q(20)) & adc_scaled_Q(20 downto 8);
				else
					adc_shifted8_Q <= adc_scaled_Q;
				end if;
				if(SHIFT_Q(2)='1') then
					adc_shifted4_Q <= (20 downto 17 => adc_shifted8_Q(20)) & adc_shifted8_Q(20 downto 4);
				else
					adc_shifted4_Q <= adc_shifted8_Q;
				end if;
				if(SHIFT_Q(1)='1') then
					adc_shifted2_Q <= (20 downto 19 => adc_shifted4_Q(20)) & adc_shifted4_Q(20 downto 2);
				else
					adc_shifted2_Q <= adc_shifted4_Q;
				end if;
				if(SHIFT_Q(0)='1') then
					adc_shifted1_Q <= adc_shifted2_Q(20) & adc_shifted2_Q(20 downto 1);
				else
					adc_shifted1_Q <= adc_shifted2_Q;
				end if;
				-- constrain values
				if(adc_shifted1_I(20 downto 9)="000000000000" or adc_shifted1_I(20 downto 9)="111111111111") then
					adc_constrained_I <= adc_shifted1_I(9 downto 0);
					CLIPPED_I <= '0';
				else
					adc_constrained_I <= adc_shifted1_I(20) & (8 downto 0 => not adc_shifted1_I(20));
					CLIPPED_I <= '1';
				end if;
				if(adc_shifted1_Q(20 downto 9)="000000000000" or adc_shifted1_Q(20 downto 9)="111111111111") then
					adc_constrained_Q <= adc_shifted1_Q(9 downto 0);
					CLIPPED_Q <= '0';
				else
					adc_constrained_Q <= adc_shifted1_Q(20) & (8 downto 0 => not adc_shifted1_Q(20));
					CLIPPED_Q <= '1';
				end if;
			end if;
		end if;
	end process;



end Behavioral;

