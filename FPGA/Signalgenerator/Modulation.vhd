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
use work.types.all;

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
				-- 00001100 QAM modulation
				-- 00001101 QAM modulation, differential
           MODTYPE : in  STD_LOGIC_VECTOR (7 downto 0);
			  -- Setting1:
			  -- 		AM: modulation depth
			  --		FM: frequency deviation
			  -- 		QAM:  Bitmask for I/Q lookup table (lower 8 bits)
			  --				Samples per Symbol (higher 8 bits)
           SETTING1 : in  STD_LOGIC_VECTOR (15 downto 0);
			  -- Setting2:
			  --		QAM: Samplerate pinc (lower word)
           SETTING2 : in  STD_LOGIC_VECTOR (15 downto 0);
			  --		QAM: Samplerate pinc (upper word)
			  SETTING3 : in STD_LOGIC_VECTOR (15 downto 0);
			  
			  I_Q_Address : in STD_LOGIC_VECTOR (8 downto 0);
			  I_Q_Data : in STD_LOGIC_VECTOR (11 downto 0);
			  I_Q_Write : in STD_LOGIC_VECTOR (0 downto 0);
			  
			  FIR_COEFF_WRITE : std_logic
			  );
end Modulation;

architecture Behavioral of Modulation is
	constant FIR_TAPS : integer := 32;
	constant FIR_MULTIPLEXED : integer := 8;
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
	COMPONENT IQMemory
	PORT (
		clka : IN STD_LOGIC;
		wea : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
		addra : IN STD_LOGIC_VECTOR(8 DOWNTO 0);
		dina : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
		clkb : IN STD_LOGIC;
		addrb : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		doutb : OUT STD_LOGIC_VECTOR(23 DOWNTO 0)
	);
	END COMPONENT;
	COMPONENT FIR
	generic (
		Taps : integer;
		Multiplexed : integer
	);
	PORT(
		CLK : IN  std_logic;
		RESET : IN  std_logic;
		NEW_DATA : IN  std_logic;
		DATA : IN  signed(11 downto 0);
		OUTPUT : OUT  signed(11 downto 0);
		COEFF_ARRAY : IN  firarray(0 to Taps-1)
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
	signal I_Q_last : std_logic_vector(11 downto 0);
	signal I_Q_index : std_logic_vector(7 downto 0);
	signal I_Q_lookup : std_logic_vector(23 downto 0);
	
	signal QAM_SPS : unsigned(7 downto 0);
	signal QAM_DDS_last_sign : std_logic;
	signal QAM_FIR_I_input : signed(11 downto 0);
	signal QAM_FIR_Q_input : signed(11 downto 0);
	signal QAM_FIR_NEW_SAMPLE : std_logic;
	signal QAM_FIR_I_output : signed(11 downto 0);
	signal QAM_FIR_Q_output : signed(11 downto 0);
	signal QAM_FIR_RESET : std_logic;
	
	signal COEFF_ARRAY : firarray(0 to FIR_TAPS-1);
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
	
	IQMem : IQMemory
	PORT MAP (
		clka => CLK,
		wea => I_Q_write,
		addra => I_Q_Address,
		dina => I_Q_Data,
		clkb => CLK,
		addrb => I_Q_index,
		doutb => I_Q_lookup
	);
	
	I_FIR: FIR
	generic map(
		Taps => FIR_TAPS,
		Multiplexed => FIR_MULTIPLEXED
	)	
	PORT MAP (
		CLK => CLK,
		RESET => QAM_FIR_RESET,
		NEW_DATA => QAM_FIR_NEW_SAMPLE,
		DATA => QAM_FIR_I_input,
		OUTPUT => QAM_FIR_I_output,
		COEFF_ARRAY => COEFF_ARRAY
	);
	
	Q_FIR: FIR
	generic map(
		Taps => FIR_TAPS,
		Multiplexed => FIR_MULTIPLEXED
	)	
	PORT MAP (
		CLK => CLK,
		RESET => QAM_FIR_RESET,
		NEW_DATA => QAM_FIR_NEW_SAMPLE,
		DATA => QAM_FIR_Q_input,
		OUTPUT => QAM_FIR_Q_output,
		COEFF_ARRAY => COEFF_ARRAY
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
			I_Q_last <= (others => '0');
			I_Q_index <= (others => '0');
			QAM_SPS <= (others => '0');
			QAM_DDS_last_sign <= '0';
			QAM_FIR_NEW_SAMPLE <= '0';
			QAM_FIR_RESET <= '1';
		elsif rising_edge(CLK) then
			if(FIR_COEFF_WRITE = '1') then
				-- received a new FIR coefficient
				-- Address will be in I_Q_Address and data in I_Q_data
				COEFF_ARRAY(to_integer(unsigned(I_Q_Address))) <= signed(I_Q_data);
			end if;
			case MODTYPE is
				-- modulation is disabled
				when "00000000" =>
					mult_enabled <= '0';
					fm_dds_enabled <= '0';
					fm_pinc <= (others => '0');
					DAC_I <= "111111111111";
					DAC_Q <= "100000000000";
					I_Q_last <= (others => '0');
					QAM_SPS <= (others => '0');
					QAM_DDS_last_sign <= '0';
					QAM_FIR_NEW_SAMPLE <= '0';
					QAM_FIR_RESET <= '1';
					QAM_SPS <= (others => '0');
				-- FM modulation
				when "00000100" =>
					mult_enabled <= '1';
					fm_dds_enabled <= '1';
					fm_pinc <= "0000" & mult_result;
					DAC_I <= not fm_sine(11) & fm_sine(10 downto 0);
					DAC_Q <= not fm_sine(11) & fm_sine(10 downto 0);
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
				-- QAM modulation
				when "00001100" | "00001101" =>
					mult_enabled <= '0';
					fm_dds_enabled <= '1';
					QAM_FIR_RESET <= '0';
					fm_pinc <= SETTING3 & SETTING2;
					if fm_sine(11)='1' and QAM_DDS_last_sign='0' then
						-- it is time for the next sample
						if(QAM_SPS = 0) then
							-- insert an actual new sample
							QAM_FIR_I_input <= signed(I_Q_lookup(11 downto 0));
							QAM_FIR_Q_input <= signed(I_Q_lookup(23 downto 12));
							-- update index of next sample
							-- set correct index in I/Q lookup RAM
							if MODTYPE = "00001100" then
								-- absolute modulation mode
								I_Q_index <= SOURCE(7 downto 0) and SETTING1(7 downto 0);
							else
								-- differential modulation mode
								I_Q_last <= std_logic_vector(unsigned(I_Q_last) + unsigned(SOURCE));
								I_Q_index <= I_Q_last(7 downto 0) and SETTING1(7 downto 0);
							end if;
							QAM_SPS <= unsigned(SETTING1(15 downto 8));
						else
							-- insert dummy zero sample
							QAM_FIR_I_input <= (others => '0');
							QAM_FIR_Q_input <= (others => '0');
							QAM_SPS <= QAM_SPS - 1;
						end if;
						QAM_FIR_NEW_SAMPLE <= '1';
					else
						QAM_FIR_NEW_SAMPLE <= '0';
					end if;
					QAM_DDS_last_sign <= fm_sine(11);

					DAC_I <= not std_logic(QAM_FIR_I_output(11)) & std_logic_vector(QAM_FIR_I_output(10 downto 0));
					DAC_Q <= not std_logic(QAM_FIR_Q_output(11)) & std_logic_vector(QAM_FIR_Q_output(10 downto 0));
				when others =>
					
			end case;
		end if;
	end process;
end Behavioral;

