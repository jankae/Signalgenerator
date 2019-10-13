--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   16:59:38 09/18/2019
-- Design Name:   
-- Module Name:   /home/jan/Projekte/Signalgenerator/FPGA/Signalgenerator/ExtSPITest.vhd
-- Project Name:  Signalgenerator
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: top
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
 
ENTITY ExtSPITest IS
END ExtSPITest;
 
ARCHITECTURE behavior OF ExtSPITest IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT top
    PORT(
         CLK : IN  std_logic;
         LED : INOUT  std_logic_vector(4 downto 0);
         SW1_CTL : INOUT  std_logic_vector(2 downto 0);
         SW2_CTL : INOUT  std_logic_vector(2 downto 0);
         MOD_EN : INOUT  std_logic;
         DORI : OUT  std_logic;
         SELIQ : OUT  std_logic;
         I : OUT  std_logic_vector(11 downto 0);
         Q : OUT  std_logic_vector(11 downto 0);
         CLK_DAC_P : OUT  std_logic;
         CLK_DAC_N : OUT  std_logic;
         REF_CLK : IN  std_logic;
         SPI_INT_SCK : IN  std_logic;
         SPI_INT_CS : IN  std_logic;
         SPI_INT_MOSI : IN  std_logic;
         SPI_INT_MISO : OUT  std_logic;
         SPI_EXT_SCK : IN  std_logic;
         SPI_EXT_CS : IN  std_logic;
         SPI_EXT_MOSI : IN  std_logic;
         SPI_EXT_MISO : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal REF_CLK : std_logic := '0';
   signal SPI_INT_SCK : std_logic := '0';
   signal SPI_INT_CS : std_logic := '0';
   signal SPI_INT_MOSI : std_logic := '0';
   signal SPI_EXT_SCK : std_logic := '0';
   signal SPI_EXT_CS : std_logic := '0';
   signal SPI_EXT_MOSI : std_logic := '0';

	--BiDirs
   signal LED : std_logic_vector(4 downto 0);
   signal SW1_CTL : std_logic_vector(2 downto 0);
   signal SW2_CTL : std_logic_vector(2 downto 0);
   signal MOD_EN : std_logic;

 	--Outputs
   signal DORI : std_logic;
   signal SELIQ : std_logic;
   signal I : std_logic_vector(11 downto 0);
   signal Q : std_logic_vector(11 downto 0);
   signal CLK_DAC_P : std_logic;
   signal CLK_DAC_N : std_logic;
   signal SPI_INT_MISO : std_logic;
   signal SPI_EXT_MISO : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 10 ns;
   constant CLK_DAC_P_period : time := 10 ns;
   constant CLK_DAC_N_period : time := 10 ns;
   constant REF_CLK_period : time := 10 ns;
 
	constant SPI_CLK_period : time := 100ns;
	
	signal data_signal : std_logic_vector(15 downto 0);
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: top PORT MAP (
          CLK => CLK,
          LED => LED,
          SW1_CTL => SW1_CTL,
          SW2_CTL => SW2_CTL,
          MOD_EN => MOD_EN,
          DORI => DORI,
          SELIQ => SELIQ,
          I => I,
          Q => Q,
          CLK_DAC_P => CLK_DAC_P,
          CLK_DAC_N => CLK_DAC_N,
          REF_CLK => REF_CLK,
          SPI_INT_SCK => SPI_INT_SCK,
          SPI_INT_CS => SPI_INT_CS,
          SPI_INT_MOSI => SPI_INT_MOSI,
          SPI_INT_MISO => SPI_INT_MISO,
          SPI_EXT_SCK => SPI_EXT_SCK,
          SPI_EXT_CS => SPI_EXT_CS,
          SPI_EXT_MOSI => SPI_EXT_MOSI,
          SPI_EXT_MISO => SPI_EXT_MISO
        );

   -- Clock process definitions
   CLK_process :process
   begin
		CLK <= '0';
		wait for CLK_period/2;
		CLK <= '1';
		wait for CLK_period/2;
   end process;
 
   CLK_DAC_P_process :process
   begin
		CLK_DAC_P <= '0';
		wait for CLK_DAC_P_period/2;
		CLK_DAC_P <= '1';
		wait for CLK_DAC_P_period/2;
   end process;
 
   CLK_DAC_N_process :process
   begin
		CLK_DAC_N <= '0';
		wait for CLK_DAC_N_period/2;
		CLK_DAC_N <= '1';
		wait for CLK_DAC_N_period/2;
   end process;
 
   REF_CLK_process :process
   begin
		REF_CLK <= '0';
		wait for REF_CLK_period/2;
		REF_CLK <= '1';
		wait for REF_CLK_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
		procedure SPI_ext(data : std_logic_vector(15 downto 0)) is
		begin
			SPI_EXT_MOSI <= data(15);
			data_signal <= data(14 downto 0) & "0";
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
			SPI_EXT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_EXT_SCK <= '0';
		end procedure SPI_ext;
		procedure SPI_int(data : std_logic_vector(15 downto 0)) is
		begin
			SPI_INT_MOSI <= data(15);
			data_signal <= data(14 downto 0) & "0";
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
			SPI_INT_MOSI <= data_signal(15);
			data_signal <= data_signal(14 downto 0) & '0';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '1';
			wait for SPI_CLK_period/2;
			SPI_INT_SCK <= '0';
		end procedure SPI_int;
   begin		
      -- hold reset state for 100 ns.
      wait for 100 ns;	

      wait for CLK_period*10;

      -- insert stimulus here
		SPI_EXT_CS <= '1';
		SPI_INT_CS <= '1';
		-- disable modulation
		wait for CLK_period*10;
		SPI_INT_CS <= '0';
		wait for CLK_period*10;
		SPI_int("1000000000000111");
		SPI_int("0000000000000000");
		wait for CLK_period*10;
		SPI_INT_CS <= '1';
		
		wait for CLK_period*10;
		-- Write first I/Q entry in lookup table
		SPI_EXT_CS <= '0';
		wait for CLK_period*10;
		SPI_ext("0000000000000000");
		SPI_ext("0000011100001111");
		SPI_ext("0000000011110000");
		wait for CLK_period*10;
		SPI_EXT_CS <= '1';
		wait for CLK_period*10;
		-- Write FIR coefficients
		SPI_EXT_CS <= '0';
		wait for CLK_period*10;
		SPI_ext("1000000000000000");
		SPI_ext("0000011111111111");
		SPI_ext("0000001111111111");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		SPI_ext("0000000000000000");
		wait for CLK_period*10;
		SPI_EXT_CS <= '1';
		
		-- set high sample rate
		wait for CLK_period*10;
		SPI_INT_CS <= '0';
		wait for CLK_period*10;
		SPI_int("1000000000000110");
		SPI_int("1111111111111111");
		wait for CLK_period*10;
		SPI_INT_CS <= '1';
		wait for CLK_period*10;
		SPI_INT_CS <= '0';
		wait for CLK_period*10;
		SPI_int("1000000000001000");
		SPI_int("0001111111111111");
		wait for CLK_period*10;
		SPI_INT_CS <= '1';
		
		-- only one valid bit, 4 samples per symbol
		wait for CLK_period*10;
		SPI_INT_CS <= '0';
		wait for CLK_period*10;
		SPI_int("1000000000000101");
		SPI_int("0000010000000001");
		wait for CLK_period*10;
		SPI_INT_CS <= '1';
		-- set modulation to QAM
		wait for CLK_period*10;
		SPI_INT_CS <= '0';
		wait for CLK_period*10;
		SPI_int("1000000000000111");
		SPI_int("0000000000001100");
		wait for CLK_period*10;
		SPI_INT_CS <= '1';

      wait;
   end process;

END;
