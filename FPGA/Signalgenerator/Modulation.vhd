----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    14:32:10 07/05/2019 
-- Design Name: 
-- Module Name:    Modulation - Behavioral 
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
use IEEE.numeric_std.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity Modulation is
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           DAC_I : out  STD_LOGIC_VECTOR (11 downto 0);
           DAC_Q : out  STD_LOGIC_VECTOR (11 downto 0);
           SOURCE : in  STD_LOGIC_VECTOR (11 downto 0);
			  	-- Modulation types:
				-- 00000000 Modulation disabled
				-- 00000100 FM modulation
				-- 00000101 FM modulation, lower sideband
				-- 00000110 FM modulation, upper sideband
				-- 00001000 AM modulation
           MODTYPE : in  STD_LOGIC_VECTOR (7 downto 0);
           SETTING1 : in  STD_LOGIC_VECTOR (15 downto 0);
           SETTING2 : in  STD_LOGIC_VECTOR (15 downto 0));
end Modulation;

architecture Behavioral of Modulation is
	COMPONENT DDS
	PORT (
		ce : IN STD_LOGIC;
		clk : IN STD_LOGIC;
		sclr : IN STD_LOGIC;
		pinc_in : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
		cosine : OUT STD_LOGIC_VECTOR(11 DOWNTO 0);
		sine : OUT STD_LOGIC_VECTOR(11 DOWNTO 0)
	);
	END COMPONENT;
	COMPONENT ModMult
	PORT (
		clk : IN STD_LOGIC;
		a : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
		b : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
		ce : IN STD_LOGIC;
		sclr : IN STD_LOGIC;
		p : OUT STD_LOGIC_VECTOR(27 DOWNTO 0)
	);
	END COMPONENT;
	signal mult_result : std_logic_vector(27 downto 0);
	signal fm_sine : std_logic_vector(11 downto 0);
	signal fm_cosine : std_logic_vector(11 downto 0);
	
	signal mult_enabled : std_logic;
	signal mult_reset : std_logic;
	signal fm_dds_enabled : std_logic;
	signal fm_dds_reset : std_logic;
	
	signal fm_pinc : std_logic_vector(31 downto 0);
begin

	FM_DDS : DDS
	PORT MAP (
		ce => fm_dds_enabled,
		clk => CLK,
		sclr => fm_dds_reset,
		pinc_in => fm_pinc,
		cosine => fm_cosine,
		sine => fm_sine
	);

	AM_FM_Mult : ModMult
	PORT MAP (
		clk => CLK,
		a => SOURCE,
		b => SETTING1,
		ce => mult_enabled,
		sclr => mult_reset,
		p => mult_result
	);
	
	fm_dds_reset <= not fm_dds_enabled;
	mult_reset <= not mult_enabled;

	process(CLK, RESET)
	begin
		if(RESET = '1') then
			mult_enabled <= '0';
			fm_dds_enabled <= '0';
			DAC_I <= "100000000000";
			DAC_Q <= "100000000000";
			fm_pinc <= (others => '0');
		elsif rising_edge(CLK) then
			case MODTYPE is
				-- modulation is disabled
				when "00000000" =>
					mult_enabled <= '0';
					fm_dds_enabled <= '0';
					fm_pinc <= (others => '0');
					DAC_I <= "111111111111";
					DAC_Q <= "100000000000";
				-- FM modulation
				when "00000100" =>
					mult_enabled <= '1';
					fm_dds_enabled <= '1';
					fm_pinc <= "0000" & mult_result;
					DAC_I <= not fm_sine(11) & fm_sine(10 downto 0);
					DAC_Q <= "100000000000";
				-- FM lower sideband modulation
				when "00000101" =>
					mult_enabled <= '1';
					fm_dds_enabled <= '1';
					fm_pinc <= "0000" & mult_result;
					DAC_I <= not fm_sine(11) & fm_sine(10 downto 0);
					DAC_Q <= not fm_cosine(11) & fm_cosine(10 downto 0);
				-- FM upper sideband modulation
				when "00000110" =>
					mult_enabled <= '1';
					fm_dds_enabled <= '1';
					fm_pinc <= "0000" & mult_result;
					DAC_I <= not fm_sine(11) & fm_sine(10 downto 0);
					DAC_Q <= fm_cosine(11) & not fm_cosine(10 downto 0);
				-- AM modulation
				when "00001000" =>
					mult_enabled <= '1';
					fm_dds_enabled <= '0';
					fm_pinc <= (others => '0');
					DAC_I <= std_logic_vector(to_unsigned(4095, 12) - unsigned(mult_result(27 downto 17)));
					DAC_Q <= "100000000000";
				when others =>
					
			end case;
		end if;
	end process;
end Behavioral;

