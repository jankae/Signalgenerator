--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   16:15:47 10/12/2019
-- Design Name:   
-- Module Name:   /home/jan/Projekte/Signalgenerator/FPGA/Signalgenerator/FIRtest.vhd
-- Project Name:  Signalgenerator
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: FIR
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
use work.types.all;

ENTITY FIRtest IS
END FIRtest;
 
ARCHITECTURE behavior OF FIRtest IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT FIR
	 	generic (
--		Datawidth : integer;
--		CoeffWidth : integer;
		Taps : integer;
		Multiplexed : integer
		);
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         NEW_DATA : IN  std_logic;
         DATA : IN  signed(11 downto 0);
         OUTPUT : OUT  signed(11 downto 0);
         COEFF_ARRAY : IN  firarray(0 to 3)
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal NEW_DATA : std_logic := '0';
   signal DATA : signed(11 downto 0) := (others => '0');
   signal COEFF_ARRAY : firarray(0 to 3) := (others => to_signed(0, 12));

 	--Outputs
   signal OUTPUT : signed(11 downto 0);

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
	constant CLKS_PER_SAMPLE : integer := 4;
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: FIR
	generic map(
		Taps => 4,
		Multiplexed => 4
	)
	
	PORT MAP (
          CLK => CLK,
          RESET => RESET,
          NEW_DATA => NEW_DATA,
          DATA => DATA,
          OUTPUT => OUTPUT,
          COEFF_ARRAY => COEFF_ARRAY
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
		RESET <= '1';
      wait for CLK_period*10;
		RESET <= '0';
      -- insert stimulus here
		COEFF_ARRAY(0) <= to_signed(2047, 12);
		COEFF_ARRAY(1) <= to_signed(1024, 12);
		COEFF_ARRAY(2) <= to_signed(512, 12);
		COEFF_ARRAY(3) <= to_signed(256, 12);
		DATA <= to_signed(2047, 12);
		wait for CLK_period*10;
		NEW_DATA <= '1';
		wait for CLK_period*1;
		NEW_DATA <= '0';
		wait for CLK_period*(CLKS_PER_SAMPLE-1);

		DATA <= to_signed(0, 12);

		
		NEW_DATA <= '1';
		wait for CLK_period;
		NEW_DATA <= '0';
		wait for CLK_period*(CLKS_PER_SAMPLE-1);



		NEW_DATA <= '1';
		wait for CLK_period;
		NEW_DATA <= '0';
		wait for CLK_period*(CLKS_PER_SAMPLE-1);
		NEW_DATA <= '1';
		wait for CLK_period;
		NEW_DATA <= '0';
		wait for CLK_period*(CLKS_PER_SAMPLE-1);
		NEW_DATA <= '1';
		wait for CLK_period;
		NEW_DATA <= '0';
		wait for CLK_period*(CLKS_PER_SAMPLE-1);
		NEW_DATA <= '1';
		wait for CLK_period;
		NEW_DATA <= '0';
		wait for CLK_period*(CLKS_PER_SAMPLE-1);
		NEW_DATA <= '1';
		wait for CLK_period;
		NEW_DATA <= '0';
		wait for CLK_period*(CLKS_PER_SAMPLE-1);
		NEW_DATA <= '1';
		wait for CLK_period;
		NEW_DATA <= '0';
		wait for CLK_period*(CLKS_PER_SAMPLE-1);


      wait;
   end process;

END;
