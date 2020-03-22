--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   18:03:46 10/20/2019
-- Design Name:   
-- Module Name:   /home/jan/Projekte/Signalgenerator/FPGA/Signalgenerator/pca9555_test.vhd
-- Project Name:  Signalgenerator
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: PCA9555
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
 
ENTITY pca9555_test IS
END pca9555_test;
 
ARCHITECTURE behavior OF pca9555_test IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT PCA9555
	 GENERIC(
			CLK_FREQ : integer;
			ADDRESS : std_logic_vector(6 downto 0)
	 );
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         SCL : OUT  std_logic;
         SDA : INOUT  std_logic;
         GPO : IN  std_logic_vector(15 downto 0);
         UPDATED : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal GPO : std_logic_vector(15 downto 0) := (others => '0');

	--BiDirs
   signal SDA : std_logic;

 	--Outputs
   signal SCL : std_logic;
   signal UPDATED : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: PCA9555
	GENERIC MAP (
		ADDRESS => "0100000",
		CLK_FREQ => 100000000
	)
	PORT MAP (
          CLK => CLK,
          RESET => RESET,
          SCL => SCL,
          SDA => SDA,
          GPO => GPO,
          UPDATED => UPDATED
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
      wait for 100 ns;	
		RESET <= '0';
		SDA <= '1';

      wait for CLK_period*10;
		

      -- insert stimulus here
		wait until UPDATED='1';
		wait for CLK_period*10000;
		GPO <= "1010010100001111";
      wait;
   end process;

END;
