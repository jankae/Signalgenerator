--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   15:18:45 03/21/2020
-- Design Name:   
-- Module Name:   /home/jan/Projekte/Signalgenerator/FPGA/Signalgenerator/ADCScalingTest.vhd
-- Project Name:  Signalgenerator
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: ADCScaling
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY ADCScalingTest IS
END ADCScalingTest;
 
ARCHITECTURE behavior OF ADCScalingTest IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT ADCScaling
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         ADC_I : IN  std_logic_vector(9 downto 0);
         ADC_Q : IN  std_logic_vector(9 downto 0);
         NEW_IN : IN  std_logic;
         OFFSET_I : IN  std_logic_vector(9 downto 0);
         OFFSET_Q : IN  std_logic_vector(9 downto 0);
         MULT_I : IN  std_logic_vector(9 downto 0);
         MULT_Q : IN  std_logic_vector(9 downto 0);
         SHIFT_I : IN  std_logic_vector(3 downto 0);
         SHIFT_Q : IN  std_logic_vector(3 downto 0);
         OUT_I : OUT  std_logic_vector(9 downto 0);
         OUT_Q : OUT  std_logic_vector(9 downto 0);
         NEW_OUT : OUT  std_logic;
         CLIPPED_I : OUT  std_logic;
         CLIPPED_Q : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal ADC_I : std_logic_vector(9 downto 0) := (others => '0');
   signal ADC_Q : std_logic_vector(9 downto 0) := (others => '0');
   signal NEW_IN : std_logic := '0';
   signal OFFSET_I : std_logic_vector(9 downto 0) := (others => '0');
   signal OFFSET_Q : std_logic_vector(9 downto 0) := (others => '0');
   signal MULT_I : std_logic_vector(9 downto 0) := (others => '0');
   signal MULT_Q : std_logic_vector(9 downto 0) := (others => '0');
   signal SHIFT_I : std_logic_vector(3 downto 0) := (others => '0');
   signal SHIFT_Q : std_logic_vector(3 downto 0) := (others => '0');

 	--Outputs
   signal OUT_I : std_logic_vector(9 downto 0);
   signal OUT_Q : std_logic_vector(9 downto 0);
   signal NEW_OUT : std_logic;
   signal CLIPPED_I : std_logic;
   signal CLIPPED_Q : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: ADCScaling PORT MAP (
          CLK => CLK,
          RESET => RESET,
          ADC_I => ADC_I,
          ADC_Q => ADC_Q,
          NEW_IN => NEW_IN,
          OFFSET_I => OFFSET_I,
          OFFSET_Q => OFFSET_Q,
          MULT_I => MULT_I,
          MULT_Q => MULT_Q,
          SHIFT_I => SHIFT_I,
          SHIFT_Q => SHIFT_Q,
          OUT_I => OUT_I,
          OUT_Q => OUT_Q,
          NEW_OUT => NEW_OUT,
          CLIPPED_I => CLIPPED_I,
          CLIPPED_Q => CLIPPED_Q
        );

   -- Clock process definitions
   CLK_process :process
   begin
		CLK <= '0';
		wait for CLK_period/2;
		CLK <= '1';
		wait for CLK_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
		RESET <= '1';
		ADC_I <= "0001111010";
		ADC_Q <= "1111111111";
		OFFSET_I <= "0001010001";
		OFFSET_Q <= "0000000000";
		MULT_I <= "0110010000";
		MULT_Q <= "0001000000";
		SHIFT_I <= "0101";
		SHIFT_Q <= "1010";
      wait for 100 ns;	
		RESET <= '0';
      wait for CLK_period*10;

      -- insert stimulus here
		NEW_IN <= '1';
		wait for CLK_period;
		NEW_IN <= '0';
		ADC_I <= "0010000000";
		ADC_Q <= "0100000000";
		wait for CLK_period*5;
		NEW_IN <= '1';
		wait for CLK_period;
		NEW_IN <= '0';

      wait;
   end process;

END;
