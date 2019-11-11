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
           DAC_I : out  signed (11 downto 0);
           DAC_Q : out  signed (11 downto 0);
           SOURCE : in  STD_LOGIC_VECTOR (11 downto 0);
			  
			  -- connections to external I/Q sample ADC
			  EXT_I : in STD_LOGIC_VECTOR(9 downto 0);
			  EXT_Q : in STD_LOGIC_VECTOR(9 downto 0);
			  EXT_NEW_SAMPLE : in STD_LOGIC;
			  EXT_ENABLE : out STD_LOGIC;
			  
			  	-- Modulation types:
				-- 00000000 Modulation disabled
				-- 00000100 FM modulation
				-- 00001000 AM modulation
				-- 00001100 QAM modulation
				-- 00001101 QAM modulation, differential
				-- 00001110 External I/Q modulation
           MODTYPE : in  STD_LOGIC_VECTOR (7 downto 0);
			  -- Setting1:
			  -- 		AM: modulation depth
			  --		FM: frequency deviation
			  -- 		QAM:  Bitmask for I/Q lookup table (lower 8 bits)
			  --				Samples per Symbol (higher 8 bits)
           SETTING1 : in  STD_LOGIC_VECTOR (15 downto 0);
			  -- Setting2:
			  --		FM: phase offset between I and Q
			  --		QAM: Samplerate pinc (lower word)
           SETTING2 : in  STD_LOGIC_VECTOR (15 downto 0);
			  --		QAM: Samplerate pinc (upper word)
			  SETTING3 : in STD_LOGIC_VECTOR (15 downto 0);
			  
			  I_Q_Address : in STD_LOGIC_VECTOR (8 downto 0);
			  I_Q_Data : in STD_LOGIC_VECTOR (11 downto 0);
			  I_Q_Write : in STD_LOGIC_VECTOR (0 downto 0);
			  
			  FIR_COEFF_WRITE : std_logic;
			  FIR_COEFF_RELOAD : std_logic
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
		poff_in : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
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

--	COMPONENT FIR
--	generic (
--		Taps : integer;
--		Multiplexed : integer
--	);
--	PORT(
--		CLK : IN  std_logic;
--		RESET : IN  std_logic;
--		NEW_DATA : IN  std_logic;
--		DATA : IN  signed(11 downto 0);
--		OUTPUT : OUT  signed(11 downto 0);
--		COEFF_ARRAY : IN  firarray(0 to Taps-1)
--	);
--   END COMPONENT;
	component fir_ip
	port (
		sclr: in std_logic;
		clk: in std_logic;
		ce: in std_logic;
		nd: in std_logic;
		coef_ld: in std_logic;
		coef_we: in std_logic;
		coef_din: in std_logic_vector(11 downto 0);
		rfd: out std_logic;
		rdy: out std_logic;
		din_1: in std_logic_vector(11 downto 0);
		din_2: in std_logic_vector(11 downto 0);
		dout_1: out std_logic_vector(30 downto 0);
		dout_2: out std_logic_vector(30 downto 0)
	);
	end component;
	signal mod_type_buf : std_logic_vector(7 downto 0);
	
	signal mult_result : std_logic_vector(27 downto 0);
	signal fm_sine : std_logic_vector(11 downto 0);
	signal fm_cosine : std_logic_vector(11 downto 0);
	
	signal mult_enabled : std_logic;
	signal mult_reset : std_logic;
	signal fm_dds_enabled : std_logic;
	signal fm_dds_reset : std_logic;
	
	signal fm_pinc : std_logic_vector(31 downto 0);
	signal fm_phase_offset : std_logic_vector(31 downto 0);
	signal I_Q_last : std_logic_vector(7 downto 0);
	signal I_Q_index : std_logic_vector(7 downto 0);
	signal I_Q_lookup : std_logic_vector(23 downto 0);
	
	signal QAM_SPS : unsigned(7 downto 0);
	signal QAM_DDS_last_sign : std_logic;
	signal QAM_FIR_I_input : std_logic_vector(11 downto 0);
	signal QAM_FIR_Q_input : std_logic_vector(11 downto 0);
	signal QAM_FIR_NEW_SAMPLE : std_logic;
	signal QAM_FIR_I_output : std_logic_vector(30 downto 0);
	signal QAM_FIR_Q_output : std_logic_vector(30 downto 0);
--	signal QAM_FIR_ENABLED : std_logic;
	
--	signal COEFF_ARRAY : firarray(0 to FIR_TAPS-1);
begin

	FM_DDS_I : DDS
	PORT MAP (
		ce => fm_dds_enabled,
		clk => CLK,
		sclr => fm_dds_reset,
		pinc_in => fm_pinc,
		poff_in => (others => '0'),
		sine => fm_sine
	);
	
	FM_DDS_Q : DDS
	PORT MAP (
		ce => fm_dds_enabled,
		clk => CLK,
		sclr => fm_dds_reset,
		pinc_in => fm_pinc,
		poff_in => fm_phase_offset,
		sine => fm_cosine
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
	
--	I_FIR: FIR
--	generic map(
--		Taps => FIR_TAPS,
--		Multiplexed => FIR_MULTIPLEXED
--	)	
--	PORT MAP (
--		CLK => CLK,
--		RESET => QAM_FIR_RESET,
--		NEW_DATA => QAM_FIR_NEW_SAMPLE,
--		DATA => QAM_FIR_I_input,
--		OUTPUT => QAM_FIR_I_output,
--		COEFF_ARRAY => COEFF_ARRAY
--	);
--	
--	Q_FIR: FIR
--	generic map(
--		Taps => FIR_TAPS,
--		Multiplexed => FIR_MULTIPLEXED
--	)	
--	PORT MAP (
--		CLK => CLK,
--		RESET => QAM_FIR_RESET,
--		NEW_DATA => QAM_FIR_NEW_SAMPLE,
--		DATA => QAM_FIR_Q_input,
--		OUTPUT => QAM_FIR_Q_output,
--		COEFF_ARRAY => COEFF_ARRAY
--	);
	FIR : fir_ip
	port map (
		sclr => '0',
		clk => clk,
		ce => '1',
		nd => QAM_FIR_NEW_SAMPLE,
		coef_ld => FIR_COEFF_RELOAD,
		coef_we => FIR_COEFF_WRITE,
		coef_din => I_Q_Data,
		rfd => open,
		rdy => open,
		din_1 => QAM_FIR_I_input,
		din_2 => QAM_FIR_Q_input,
		dout_1 => QAM_FIR_I_output,
		dout_2 => QAM_FIR_Q_output
	);
	
	fm_dds_reset <= not fm_dds_enabled;
	mult_reset <= not mult_enabled;
	fm_phase_offset <= SETTING2 & "0000000000000000";

	process(CLK, RESET)
	begin
		if(RESET = '1') then
			mult_enabled <= '0';
			fm_dds_enabled <= '0';
			DAC_I <= (others => '0');
			DAC_Q <= (others => '0');
			fm_pinc <= (others => '0');
			I_Q_last <= (others => '0');
			I_Q_index <= (others => '0');
			QAM_SPS <= (others => '0');
			QAM_DDS_last_sign <= '0';
			QAM_FIR_NEW_SAMPLE <= '0';
			EXT_ENABLE <= '0';
		elsif rising_edge(CLK) then
--			if(FIR_COEFF_WRITE = '1') then
--				-- received a new FIR coefficient
--				-- Address will be in I_Q_Address and data in I_Q_data
--				COEFF_ARRAY(to_integer(unsigned(I_Q_Address))) <= signed(I_Q_data);
--			end if;
			mod_type_buf <= MODTYPE;
			case mod_type_buf is
				-- modulation is disabled
				when "00000000" =>
					mult_enabled <= '0';
					fm_dds_enabled <= '0';
					fm_pinc <= (others => '0');
					DAC_I <= "011111111111";
					DAC_Q <= "000000000000";
					I_Q_last <= (others => '0');
					QAM_SPS <= (others => '0');
					QAM_DDS_last_sign <= '0';
					QAM_FIR_NEW_SAMPLE <= '0';
					QAM_SPS <= (others => '0');
					EXT_ENABLE <= '0';
				-- FM modulation
				when "00000100" =>
					mult_enabled <= '1';
					fm_dds_enabled <= '1';
					fm_pinc <= "0000" & mult_result;
					DAC_I <= signed(fm_sine);
					DAC_Q <= signed(fm_cosine);
					EXT_ENABLE <= '0';
				-- AM modulation
				when "00001000" =>
					mult_enabled <= '1';
					fm_dds_enabled <= '0';
					fm_pinc <= (others => '0');
					DAC_I <= signed(to_unsigned(2047, 12) - unsigned(mult_result(27 downto 17)));
					DAC_Q <= (others => '0');
					EXT_ENABLE <= '0';
				-- QAM modulation
				when "00001100" | "00001101" =>
					EXT_ENABLE <= '0';
					mult_enabled <= '0';
					fm_dds_enabled <= '1';
					fm_pinc <= SETTING3 & SETTING2;
					if fm_sine(11)='1' and QAM_DDS_last_sign='0' then
						-- it is time for the next sample
						if(QAM_SPS = 0) then
							-- insert an actual new sample
							QAM_FIR_I_input <= I_Q_lookup(11 downto 0);
							QAM_FIR_Q_input <= I_Q_lookup(23 downto 12);
							-- update index of next sample
							-- set correct index in I/Q lookup RAM
							if mod_type_buf = "00001100" then
								-- absolute modulation mode
								I_Q_index <= SOURCE(7 downto 0) and SETTING1(7 downto 0);
							else
								-- differential modulation mode
								I_Q_last <= std_logic_vector(unsigned(I_Q_last) + unsigned(SOURCE(7 downto 0)));
								I_Q_index <= I_Q_last(7 downto 0) and SETTING1(7 downto 0);
							end if;
							QAM_SPS <= unsigned(SETTING1(15 downto 8)) - 1;
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

					DAC_I <= signed(QAM_FIR_I_output(23 downto 12));
					DAC_Q <= signed(QAM_FIR_Q_output(23 downto 12));
				-- external modulation
				when "00001110" =>
					-- enable sampling of ADC, pass samples through FIR and send to DAC
					EXT_ENABLE <= '1';
					QAM_FIR_I_input <= EXT_I & "00";
					QAM_FIR_Q_input <= EXT_Q & "00";
					QAM_FIR_NEW_SAMPLE <= EXT_NEW_SAMPLE;
					DAC_I <= signed(QAM_FIR_I_output(23 downto 12));
					DAC_Q <= signed(QAM_FIR_Q_output(23 downto 12));
				when others =>
					
			end case;
		end if;
	end process;
end Behavioral;

