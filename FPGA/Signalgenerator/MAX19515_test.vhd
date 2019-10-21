--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   10:13:29 10/21/2019
-- Design Name:   
-- Module Name:   /home/jan/Projekte/Signalgenerator/FPGA/Signalgenerator/MAX19515_test.vhd
-- Project Name:  Signalgenerator
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: MAX19515
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
USE ieee.numeric_std.ALL;
 
ENTITY MAX19515_test IS
END MAX19515_test;
 
ARCHITECTURE behavior OF MAX19515_test IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT MAX19515
	 Generic (
		SampleEveryNthCLK : integer
	 );
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         SAMPLE_CLK : OUT  std_logic;
         DCLK : IN  std_logic;
         DATA : IN  std_logic_vector(9 downto 0);
         OUT_A : OUT  std_logic_vector(9 downto 0);
         OUT_B : OUT  std_logic_vector(9 downto 0);
         NEW_SAMPLE : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal DCLK : std_logic := '0';
   signal DATA : std_logic_vector(9 downto 0) := (others => '0');

 	--Outputs
   signal SAMPLE_CLK : std_logic;
   signal OUT_A : std_logic_vector(9 downto 0);
   signal OUT_B : std_logic_vector(9 downto 0);
   signal NEW_SAMPLE : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
   constant SAMPLE_CLK_period : time := 10 ns;
   constant DCLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: MAX19515
	GENERIC MAP(
			SampleEveryNthCLK => 6
	)
	PORT MAP (
          CLK => CLK,
          RESET => RESET,
          ADC_SAMPLE_CLK => SAMPLE_CLK,
          ADC_DCLK => DCLK,
          ADC_DATA => DATA,
          OUT_A => OUT_A,
          OUT_B => OUT_B,
          NEW_SAMPLE => NEW_SAMPLE
        );

   -- Clock process definitions
   CLK_process :process
   begin
		CLK <= '0';
		wait for CLK_period/2;
		CLK <= '1';
		wait for CLK_period/2;
   end process;
 
   DATA_process :process
   begin
		wait until SAMPLE_CLK = '1';
		wait for 4ns;
		DATA <= std_logic_vector( unsigned(DATA) + 1 );
		wait until SAMPLE_CLK = '0';
		wait for 4ns;
		DATA <= std_logic_vector( unsigned(DATA) + 1 );
   end process;
 
   DCLK_process :process
   begin
		wait until SAMPLE_CLK = '1';
		wait for 2.2ns;
		DCLK <= '1';
		wait until SAMPLE_CLK = '0';
		wait for 2.2ns;
		DCLK <= '0';
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
      RESET <= '1';
		wait for 100 ns;	
		RESET <= '0';
      wait for CLK_period*10;

      -- insert stimulus here 

      wait;
   end process;

END;
