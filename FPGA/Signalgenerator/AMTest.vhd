--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   20:39:30 07/05/2019
-- Design Name:   
-- Module Name:   /home/jan/Projekte/Signalgenerator/FPGA/Signalgenerator/AMTest.vhd
-- Project Name:  Signalgenerator
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: Modulation
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
 
ENTITY AMTest IS
END AMTest;
 
ARCHITECTURE behavior OF AMTest IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT Modulation
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         DAC_I : OUT  std_logic_vector(11 downto 0);
         DAC_Q : OUT  std_logic_vector(11 downto 0);
         SOURCE : IN  std_logic_vector(11 downto 0);
         MODTYPE : IN  std_logic_vector(7 downto 0);
         SETTING1 : IN  std_logic_vector(15 downto 0);
         SETTING2 : IN  std_logic_vector(15 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal SOURCE : std_logic_vector(11 downto 0) := (others => '0');
   signal MODTYPE : std_logic_vector(7 downto 0) := (others => '0');
   signal SETTING1 : std_logic_vector(15 downto 0) := (others => '0');
   signal SETTING2 : std_logic_vector(15 downto 0) := (others => '0');

 	--Outputs
   signal DAC_I : std_logic_vector(11 downto 0);
   signal DAC_Q : std_logic_vector(11 downto 0);

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: Modulation PORT MAP (
          CLK => CLK,
          RESET => RESET,
          DAC_I => DAC_I,
          DAC_Q => DAC_Q,
          SOURCE => SOURCE,
          MODTYPE => MODTYPE,
          SETTING1 => SETTING1,
          SETTING2 => SETTING2
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
      wait for 100 ns;	

      wait for CLK_period*10;

      -- insert stimulus here 
		MODTYPE <= "00001000";
		SOURCE <= "000000000000";
		SETTING1 <= "0000000000000000";
		wait for CLK_period*10;
		SOURCE <= "111111111111";
		SETTING1 <= "0000000000000000";
		wait for CLK_period*10;
		SOURCE <= "000000000001";
		SETTING1 <= "1111111111111111";
		wait for CLK_period*10;
		SOURCE <= "111111111111";
		SETTING1 <= "1111111111111111";
		wait for CLK_period*10;
      wait;
   end process;

END;
