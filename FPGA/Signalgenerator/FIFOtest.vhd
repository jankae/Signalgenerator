--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   15:36:22 07/13/2019
-- Design Name:   
-- Module Name:   /home/jan/Projekte/Signalgenerator/FPGA/Signalgenerator/FIFOtest.vhd
-- Project Name:  Signalgenerator
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: FIFO
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
 
ENTITY FIFOtest IS
END FIFOtest;
 
ARCHITECTURE behavior OF FIFOtest IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT FIFO
	 	generic (
		Datawidth : integer;
		Addresswidth : integer
	);
    PORT(
         CLK : IN  std_logic;
         DIN : IN  std_logic_vector(8-1 downto 0);
         DOUT : OUT  std_logic_vector(8-1 downto 0);
         WR : IN  std_logic;
         RD : IN  std_logic;
         LEVEL : OUT  std_logic_vector(2-1 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal DIN : std_logic_vector(8-1 downto 0) := (others => '0');
   signal DOUT : std_logic_vector(8-1 downto 0) := (others => '0');
   signal WR : std_logic := '0';
   signal RD : std_logic := '0';

 	--Outputs
   signal LEVEL : std_logic_vector(2-1 downto 0);

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: FIFO 
	
		GENERIC MAP (
			Addresswidth => 2,
			Datawidth => 8
		)
		PORT MAP (
          CLK => CLK,
          DIN => DIN,
          DOUT => DOUT,
          WR => WR,
          RD => RD,
          LEVEL => LEVEL
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
		DIN <= "00001111";
		WR <= '1';
		wait for CLK_period;
		WR <= '0';
		wait for CLK_period*5;
		
		DIN <= "11110000";
		WR <= '1';
		wait for CLK_period;
		WR <= '0';
		wait for CLK_period*5;
		
		DIN <= "10101010";
		WR <= '1';
		wait for CLK_period;
		WR <= '0';
		wait for CLK_period*5;
		
		DIN <= "01010101";
		WR <= '1';
		wait for CLK_period;
		WR <= '0';
		wait for CLK_period*5;
		
		wait for CLK_period*5;
		RD <= '1';
		wait for CLK_period;
		RD <= '0';
		wait for CLK_period*5;
		RD <= '1';
		wait for CLK_period;
		RD <= '0';
		wait for CLK_period*5;
		RD <= '1';
		wait for CLK_period;
		RD <= '0';
		wait for CLK_period*5;
		RD <= '1';
		wait for CLK_period;
		RD <= '0';

      wait;
   end process;

END;
