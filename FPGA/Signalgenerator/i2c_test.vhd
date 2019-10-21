--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   16:56:20 10/20/2019
-- Design Name:   
-- Module Name:   /home/jan/Projekte/Signalgenerator/FPGA/Signalgenerator/i2c_test.vhd
-- Project Name:  Signalgenerator
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: i2c
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
 
ENTITY i2c_test IS
END i2c_test;
 
ARCHITECTURE behavior OF i2c_test IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT i2c
	 	generic (
		CLK_FREQ : integer;
		I2C_FREQ : integer
	);
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         START : IN  std_logic;
         STOP : IN  std_logic;
         DATA : IN  std_logic_vector(7 downto 0);
         WR : IN  std_logic;
         BUSY : OUT  std_logic;
         ACK : OUT  std_logic;
         SDA : INOUT  std_logic;
         SCL : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';
   signal START : std_logic := '0';
   signal STOP : std_logic := '0';
   signal DATA : std_logic_vector(7 downto 0) := (others => '0');
   signal WR : std_logic := '0';

	--BiDirs
   signal SDA : std_logic;

 	--Outputs
   signal BUSY : std_logic;
   signal ACK : std_logic;
   signal SCL : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: i2c
	generic map (
		CLK_FREQ => 100000000,
		I2C_FREQ => 1000000
	)
	PORT MAP (
          CLK => CLK,
          RESET => RESET,
          START => START,
          STOP => STOP,
          DATA => DATA,
          WR => WR,
          BUSY => BUSY,
          ACK => ACK,
          SDA => SDA,
          SCL => SCL
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
		RESET <= '1';
		wait for CLK_period*10;
		RESET <= '0';
		wait for CLK_period*10;
		START <= '1';
		wait for CLK_period;
		START <= '0';
		wait until BUSY = '0';
		DATA <= "10101010";
		WR <= '1';
		wait for CLK_period*1.5;
		WR <= '0';
		wait until BUSY = '0';
		STOP <= '1';
		wait for CLK_period*1.5;
		STOP <= '0';
		wait until BUSY = '0';
      wait;
   end process;

END;
