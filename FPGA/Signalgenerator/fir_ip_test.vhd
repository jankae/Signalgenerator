--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   13:00:42 10/15/2019
-- Design Name:   
-- Module Name:   /home/jan/Projekte/Signalgenerator/FPGA/Signalgenerator/fir_ip_test.vhd
-- Project Name:  Signalgenerator
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: fir_ip
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
 
ENTITY fir_ip_test IS
END fir_ip_test;
 
ARCHITECTURE behavior OF fir_ip_test IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT fir_ip
    PORT(
         sclr : IN  std_logic;
         clk : IN  std_logic;
         ce : IN  std_logic;
         nd : IN  std_logic;
         coef_ld : IN  std_logic;
         coef_we : IN  std_logic;
         coef_din : IN  std_logic_vector(11 downto 0);
         rfd : OUT  std_logic;
         rdy : OUT  std_logic;
         din_1 : IN  std_logic_vector(11 downto 0);
         din_2 : IN  std_logic_vector(11 downto 0);
         dout_1 : OUT  std_logic_vector(30 downto 0);
         dout_2 : OUT  std_logic_vector(30 downto 0)
        );
    END COMPONENT;
    

   --Inputs
   signal sclr : std_logic := '0';
   signal clk : std_logic := '0';
   signal ce : std_logic := '0';
   signal nd : std_logic := '0';
   signal coef_ld : std_logic := '0';
   signal coef_we : std_logic := '0';
   signal coef_din : std_logic_vector(11 downto 0) := (others => '0');
   signal din_1 : std_logic_vector(11 downto 0) := (others => '0');
   signal din_2 : std_logic_vector(11 downto 0) := (others => '0');

 	--Outputs
   signal rfd : std_logic;
   signal rdy : std_logic;
   signal dout_1 : std_logic_vector(30 downto 0);
	
	signal dac : std_logic_vector(11 downto 0);
   signal dout_2 : std_logic_vector(30 downto 0);

   -- Clock period definitions
   constant clk_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: fir_ip PORT MAP (
          sclr => sclr,
          clk => clk,
          ce => ce,
          nd => nd,
          coef_ld => coef_ld,
          coef_we => coef_we,
          coef_din => coef_din,
          rfd => rfd,
          rdy => rdy,
          din_1 => din_1,
          din_2 => din_2,
          dout_1 => dout_1,
          dout_2 => dout_2
        );

   -- Clock process definitions
   clk_process :process
   begin
		clk <= '0';
		wait for clk_period/2;
		clk <= '1';
		wait for clk_period/2;
   end process;
 
	dac <= dout_1(22 downto 11);

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
      wait for 100 ns;	
		sclr <= '1';
		ce <= '1';
		wait for clk_period*10;
		sclr <= '0';
  		wait for clk_period*20;
		coef_ld <= '1';
		wait for clk_period;
		coef_ld <= '0';
		coef_we <= '1';
		wait for clk_period*30;
		coef_din <= "011111111111";
		wait for clk_period;
		coef_din <= (others => '0');
		wait for clk_period*5;
		coef_we <= '0';

		wait for clk_period*10;
		-- excite filter
		din_1 <= "100000000000";
		nd <= '1';
		wait for clk_period*6;
		din_1 <= "011111111111";
		wait for clk_period*6;
		din_1 <= "000000000001";
		wait for clk_period;
		din_1 <= "000000000000";
		nd <= '0';
		wait for clk_period*5;
		for i in 0 to 100 loop
			nd <= '1';
			wait for clk_period;
			nd <= '0';
			wait for clk_period*5;
		end loop;

      wait;
   end process;

END;
