----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    10:46:42 06/30/2019 
-- Design Name: 
-- Module Name:    top - Behavioral 
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
Library UNISIM;
use UNISIM.vcomponents.all;
use IEEE.numeric_std.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity top is
    Port ( CLK : in  STD_LOGIC;
			  RESET : in STD_LOGIC;
           LED : inout  STD_LOGIC_VECTOR (4 downto 0);
           SW1_CTL : inout  STD_LOGIC_VECTOR (2 downto 0);
           SW2_CTL : inout  STD_LOGIC_VECTOR (2 downto 0);
           MOD_EN : inout  STD_LOGIC;
           DORI : out  STD_LOGIC;
           SELIQ : out  STD_LOGIC;
           I : out  STD_LOGIC_VECTOR (11 downto 0);
           Q : out  STD_LOGIC_VECTOR (11 downto 0);
           CLK_DAC_P : out  STD_LOGIC;
           CLK_DAC_N : out  STD_LOGIC;
           SPI_INT_SCK : in  STD_LOGIC;
           SPI_INT_CS : in  STD_LOGIC;
           SPI_INT_MOSI : in  STD_LOGIC;
           SPI_INT_MISO : out  STD_LOGIC;
           SPI_EXT_SCK : in  STD_LOGIC;
           SPI_EXT_CS : in  STD_LOGIC;
           SPI_EXT_MOSI : in  STD_LOGIC;
           SPI_EXT_MISO : out  STD_LOGIC;
			  ADC_SAMPLE_CLK : out STD_LOGIC;
			  ADC_DATA : in STD_LOGIC_VECTOR(9 downto 0);
			  ADC_DCLK : in STD_LOGIC;
			  SCL : out STD_LOGIC;
			  SDA : inout STD_LOGIC
			  );
end top;

--FPGA register map:
--0x0000: GPIO register:
--	[0-2]: SW1_CTL
--	[4-6]: SW2_CTL
--	[7]: Modulator enabled
--	[8-12]: LEDs
--0x0001: reserved
--0x0002: DAC I direct:
--	[0-11]: I DAC value if modulation disabled
--0x0003: DAC Q direct:
--	[0-11]: Q DAC value if modulation disabled
--0x0004: Modulation source phase increment
--0x0005: Modulation setting1
--0x0007: Modulation CTRL:
--	[0-7]: Modulation type
--	[11-8]: Source type

architecture Behavioral of top is
	component MainPLL
	port
	 (-- Clock in ports
	  CLK_IN1           : in     std_logic;
	  -- Clock out ports
	  CLK_OUT1          : out    std_logic;
	  -- Status and control signals
	  RESET             : in     std_logic;
	  LOCKED            : out    std_logic
	 );
	end component;
	COMPONENT spi_slave
	generic (W : integer);
	PORT(
		SPI_CLK : IN std_logic;
		MOSI : IN std_logic;
		CS : IN std_logic;
		BUF_IN : IN std_logic_vector (W-1 downto 0);
		CLK : IN std_logic;          
		MISO : OUT std_logic;
		BUF_OUT : OUT std_logic_vector (W-1 downto 0);
		COMPLETE : OUT std_logic
	);
	END COMPONENT;
	
	COMPONENT Modulation
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		SOURCE : IN std_logic_vector(11 downto 0);
		EXT_I : in STD_LOGIC_VECTOR(9 downto 0);
		EXT_Q : in STD_LOGIC_VECTOR(9 downto 0);
		EXT_NEW_SAMPLE : in STD_LOGIC;
		EXT_ENABLE : out STD_LOGIC;
		MODTYPE : IN std_logic_vector(7 downto 0);
		SETTING1 : IN std_logic_vector(15 downto 0);
		SETTING2 : IN std_logic_vector(15 downto 0); 
	   SETTING3 : in STD_LOGIC_VECTOR (15 downto 0);		
		DAC_I : OUT signed(11 downto 0);
		DAC_Q : OUT signed(11 downto 0);
		
		I_Q_Address : in STD_LOGIC_VECTOR (8 downto 0);
		I_Q_Data : in STD_LOGIC_VECTOR (11 downto 0);
		I_Q_Write : in STD_LOGIC_VECTOR (0 downto 0);
		FIR_COEFF_WRITE : std_logic;
		FIR_COEFF_RELOAD : std_logic
		);
	END COMPONENT;
	
	COMPONENT MAX19515
	GENERIC(
		SampleEveryNthCLK : integer
	);
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		DCLK : IN std_logic;
		DATA : IN std_logic_vector(9 downto 0);          
		SAMPLE_CLK : OUT std_logic;
		OUT_A : OUT std_logic_vector(9 downto 0);
		OUT_B : OUT std_logic_vector(9 downto 0);
		NEW_SAMPLE : OUT std_logic
		);
	END COMPONENT;
	
	COMPONENT PCA9555
	GENERIC(
		CLK_FREQ : integer;
		ADDRESS : std_logic_vector(6 downto 0)
	);
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		GPO : IN std_logic_vector(15 downto 0);    
		SDA : INOUT std_logic;      
		SCL : OUT std_logic;
		UPDATED : OUT std_logic
		);
	END COMPONENT;
	
	COMPONENT ModSource
	GENERIC(
		STREAM_DEPTH : integer
	);
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		SRCTYPE : IN std_logic_vector(3 downto 0);
		PINC : IN std_logic_vector(19 downto 0);          
		RESULT : OUT std_logic_vector(11 downto 0);
		NEW_SAMPLE : OUT std_logic;
		FIFO_IN : in STD_LOGIC_VECTOR (11 downto 0);
		FIFO_WRITE : in STD_LOGIC;
		FIFO_LEVEL : out STD_LOGIC_VECTOR (STREAM_DEPTH-1 downto 0)
	);
	END COMPONENT;
	
	signal CLK100 : std_logic;
	signal CLK100_INV : std_logic;
	signal DAC_CLK : std_logic;
	
	signal dac_I_signed : signed(11 downto 0);
	signal dac_Q_signed : signed(11 downto 0);
	
	signal first : std_logic := '0';
	signal write_not_read : std_logic := '0';
	signal mem_address : std_logic_vector(14 downto 0) := (others => '0');
	
	signal spi_int_out : std_logic_vector(15 downto 0);
	signal spi_int_in : std_logic_vector(15 downto 0);
	signal spi_int_complete : std_logic := '0';
	
	signal spi_ext_out : std_logic_vector(15 downto 0);
	signal spi_ext_in : std_logic_vector(15 downto 0);
	signal spi_ext_complete : std_logic := '0';
	signal spi_ext_first_word : std_logic := '1';
	signal spi_ext_I_Q_address : std_logic_vector(8 downto 0);
	signal spi_ext_fifo_data : std_logic := '0';
	signal spi_ext_fir_data : std_logic := '0';
	
	-- modulation settings
	signal mod_type : std_logic_vector(7 downto 0);
	signal mod_setting1 : std_logic_vector(15 downto 0);
	signal mod_setting2 : std_logic_vector(15 downto 0);
	signal mod_setting3 : std_logic_vector(15 downto 0);
	
	signal mod_write_I_Q : std_logic_vector(0 downto 0) := "0";
	signal mod_write_FIR : std_logic;
	signal mod_reload_FIR : std_logic;
	
	signal mod_use_ext_adc : std_logic;
	-- modulation source settings
	signal mod_src_pinc : std_logic_vector(19 downto 0);
	signal mod_src_type : std_logic_vector(3 downto 0);
	signal mod_src_value : std_logic_vector(11 downto 0);
	
	signal mod_src_write : std_logic := '0';
	signal mod_src_fifo_level : std_logic_vector(13 downto 0);

	signal pll_locked : std_logic;
	
	-- external ADC signals
	signal EXT_ADC_SHUTDOWN : std_logic;
	signal EXT_ADC_NEW_SAMPLE : std_logic;
	signal EXT_ADC_CH_A : std_logic_vector(9 downto 0);
	signal EXT_ADC_CH_B : std_logic_vector(9 downto 0);
	
	signal EXT_PORT_VALUES : std_logic_vector(15 downto 0);
	signal EXT_PORT_UPDATED : std_logic;
	
	signal EXT_ADC_MAX : unsigned(9 downto 0);
	signal EXT_ADC_ABS_I : unsigned(9 downto 0);
	signal EXT_ADC_ABS_Q : unsigned(9 downto 0);
	signal EXT_ADC_LED_I_ENABLE : std_logic;
	signal EXT_ADC_LED_Q_ENABLE : std_logic;
	signal EXT_ADC_I_OVERRANGE : std_logic;
	signal EXT_ADC_Q_OVERRANGE : std_logic;
	signal EXT_ADC_I_OVR_CNT : integer range 0 to 134217727;
	signal EXT_ADC_Q_OVR_CNT : integer range 0 to 134217727;
begin
your_instance_name : MainPLL
  port map
   (-- Clock in ports
    CLK_IN1 => CLK,
    -- Clock out ports
    CLK_OUT1 => CLK100,
    -- Status and control signals
	 RESET  => RESET,
    LOCKED => pll_locked);
	 
	 CLK100_INV <= not CLK100;
	 
	 ODDR2_inst : ODDR2
   generic map(
      DDR_ALIGNMENT => "NONE", -- Sets output alignment to "NONE", "C0", "C1" 
      INIT => '0', -- Sets initial state of the Q output to '0' or '1'
      SRTYPE => "SYNC") -- Specifies "SYNC" or "ASYNC" set/reset
   port map (
      Q => DAC_CLK, -- 1-bit output data
      C0 => CLK100, -- 1-bit clock input
      C1 => CLK100_INV, -- 1-bit clock input
      CE => '1',  -- 1-bit clock enable input
      D0 => '0',   -- 1-bit data input (associated with C0)
      D1 => '1',   -- 1-bit data input (associated with C1)
      R => '0',    -- 1-bit reset input
      S => '0'     -- 1-bit set input
   );
	
	 OBUFDS_inst : OBUFDS
   generic map (
      IOSTANDARD => "DEFAULT")
   port map (
      O => CLK_DAC_P,     -- Diff_p output (connect directly to top-level port)
      OB => CLK_DAC_N,   -- Diff_n output (connect directly to top-level port)
      I => DAC_CLK      -- Buffer input 
   );
	
	InternalSPI : spi_slave 
	GENERIC MAP(W => 16)
	PORT MAP(
		SPI_CLK => SPI_INT_SCK,
		MISO => SPI_INT_MISO,
		MOSI => SPI_INT_MOSI,
		CS => SPI_INT_CS,
		BUF_OUT => spi_int_out,
		BUF_IN => spi_int_in,
		CLK => CLK100,
		COMPLETE => spi_int_complete
	);
	
	ExternalSPI : spi_slave 
	GENERIC MAP(W => 16)
	PORT MAP(
		SPI_CLK => SPI_EXT_SCK,
		MISO => SPI_EXT_MISO,
		MOSI => SPI_EXT_MOSI,
		CS => SPI_EXT_CS,
		BUF_OUT => spi_ext_out,
		BUF_IN => spi_ext_in,
		CLK => CLK100,
		COMPLETE => spi_ext_complete
	);
	
	Source: ModSource
	GENERIC MAP (STREAM_DEPTH => 14)
	PORT MAP(
		CLK => CLK100,
		RESET => RESET,
		SRCTYPE => mod_src_type,
		PINC => mod_src_pinc,
		RESULT => mod_src_value,
		NEW_SAMPLE => open,
		FIFO_IN => spi_ext_out(11 downto 0),
		FIFO_WRITE => mod_src_write,
		FIFO_LEVEL => mod_src_fifo_level
	);
	
	Modulator: Modulation PORT MAP(
		CLK => CLK100,
		RESET => RESET,
		DAC_I => dac_I_signed,
		DAC_Q => dac_Q_signed,
		SOURCE => mod_src_value,
		EXT_I => EXT_ADC_CH_B,
		EXT_Q => EXT_ADC_CH_A,
		EXT_NEW_SAMPLE => EXT_ADC_NEW_SAMPLE,
		EXT_ENABLE => mod_use_ext_adc,
		MODTYPE => mod_type,
		SETTING1 => mod_setting1,
		SETTING2 => mod_setting2,
		SETTING3 => mod_setting3,
		I_Q_Address => spi_ext_I_Q_address,
		I_Q_Data => spi_ext_out(11 downto 0),
		I_Q_Write => mod_write_I_Q,
		FIR_COEFF_WRITE => mod_write_FIR,
		FIR_COEFF_RELOAD => mod_reload_FIR
	);
	
	EXT_ADC: MAX19515
	GENERIC MAP (
		SampleEveryNthCLK => 6
	)
	PORT MAP(
		CLK => CLK100,
		RESET => EXT_ADC_SHUTDOWN,
		SAMPLE_CLK => ADC_SAMPLE_CLK,
		DCLK => ADC_DCLK,
		DATA => ADC_DATA,
		OUT_A => EXT_ADC_CH_A,
		OUT_B => EXT_ADC_CH_B,
		NEW_SAMPLE => EXT_ADC_NEW_SAMPLE 
	);
	
	PortExpander: PCA9555
	GENERIC MAP(
		CLK_FREQ => 200000000,
		ADDRESS => "0100000"
	)
	PORT MAP(
		CLK => CLK100,
		RESET => RESET,
		SCL => SCL,
		SDA => SDA,
		GPO => EXT_PORT_VALUES,
		UPDATED => EXT_PORT_UPDATED
	);
	
	I <= std_logic(dac_I_signed(11)) & not std_logic_vector(dac_I_signed(10 downto 0));
	Q <= std_logic(dac_Q_signed(11)) & not std_logic_vector(dac_Q_signed(10 downto 0));
	
	DORI <= '1';
	SELIQ <= '0';
	
	LED(4) <= not pll_locked;
	LED(3) <= RESET;
	LED(2 downto 0) <= (others => '0');
	
	EXT_ADC_SHUTDOWN <= not mod_use_ext_adc;
	EXT_PORT_VALUES(11) <= EXT_ADC_SHUTDOWN;
	EXT_PORT_VALUES(3) <= EXT_ADC_LED_I_ENABLE and not EXT_ADC_I_OVERRANGE;
	EXT_PORT_VALUES(2) <= EXT_ADC_LED_I_ENABLE and EXT_ADC_I_OVERRANGE;
	EXT_PORT_VALUES(5) <= EXT_ADC_LED_Q_ENABLE and not EXT_ADC_Q_OVERRANGE;
	EXT_PORT_VALUES(4) <= EXT_ADC_LED_Q_ENABLE and EXT_ADC_Q_OVERRANGE;
	-- set unused ports to 0
	EXT_PORT_VALUES(1 downto 0) <= (others => '0');
	EXT_PORT_VALUES(10 downto 6) <= (others => '0');
	
	spi_int_in <= EXT_ADC_CH_A & "00000" & EXT_PORT_UPDATED;
	
	process(CLK100)
	begin
		if rising_edge(CLK100) then
		if(RESET = '1') then
			MOD_EN <= '1';--signal is inverted, disables modulation IC
			-- select lowest LP filter
			SW1_CTL <= "101";
			SW2_CTL <= "010";
			-- set default values for all signals
			first <= '1';
			write_not_read <= '0';
			mem_address <= (others => '0');
			--spi_int_in <= (others => '0');
			spi_ext_in <= (others => '0');
			spi_ext_first_word <= '1';
			spi_ext_I_Q_address <= (others => '0');
			spi_ext_fifo_data <= '0';
			spi_ext_fir_data <= '0';
			mod_type <= (others => '0');
			mod_setting1 <= (others => '0');
			mod_setting2 <= (others => '0');
			mod_setting3 <= (others => '0');
			mod_write_I_Q <= (others => '0');
			mod_write_FIR <= '0';
			mod_reload_FIR <= '0';
			mod_src_pinc <= (others => '0');
			mod_src_type <= (others => '0');
			mod_src_write <= '0';
			ext_adc_I_ovr_cnt <= 0;
			ext_adc_Q_ovr_cnt <= 0;
			ext_adc_max <= to_unsigned(511, ext_adc_max'length);
		else
			-- keep track of overrange on external ADC
			if(EXT_ADC_CH_B = "1000000000") then
				EXT_ADC_ABS_I <= "1000000000";
			else
				EXT_ADC_ABS_I <= unsigned(abs(signed(EXT_ADC_CH_B)));
			end if;
			if(EXT_ADC_ABS_I>EXT_ADC_MAX) then
				ext_adc_I_ovr_cnt <= 134217727;
				EXT_ADC_I_OVERRANGE <= '1';
			elsif	ext_adc_I_ovr_cnt > 0 then
				ext_adc_I_ovr_cnt <= ext_adc_I_ovr_cnt - 1;
			else
				EXT_ADC_I_OVERRANGE <= '0';
			end if;
			
			if(EXT_ADC_CH_A = "1000000000") then
				EXT_ADC_ABS_Q <= "1000000000";
			else
				EXT_ADC_ABS_Q <= unsigned(abs(signed(EXT_ADC_CH_A)));
			end if;
			if(EXT_ADC_ABS_Q>EXT_ADC_MAX) then
				ext_adc_Q_ovr_cnt <= 134217727;
				EXT_ADC_Q_OVERRANGE <= '1';
			elsif	ext_adc_Q_ovr_cnt > 0 then
				ext_adc_Q_ovr_cnt <= ext_adc_Q_ovr_cnt - 1;
			else
				EXT_ADC_Q_OVERRANGE <= '0';
			end if;
		
			-- reset signals only valid for one cycle
			if mod_src_write = '1' then
				mod_src_write <= '0';
			end if;
			if mod_write_I_Q = "1" or mod_write_FIR = '1' then
				mod_write_I_Q <= "0";
				mod_write_FIR <= '0';
				spi_ext_I_Q_address <= std_logic_vector(unsigned(spi_ext_I_Q_address) + 1);
			end if;
			if mod_reload_FIR = '1' then
				mod_reload_FIR <= '0';
			end if;
			
	-----------------------------------------------
	-- START: External SPI handling
	-----------------------------------------------
				if SPI_EXT_CS = '1' then
					spi_ext_first_word <= '1';
					spi_ext_fifo_data <= '0';
					spi_ext_fir_data <= '0';
				else
					if spi_ext_complete = '1' then
						if spi_ext_first_word = '1' then
							spi_ext_first_word <= '0';
							-- this is the first word, determine if transfer is
							-- for modulation source FIFO or for I/Q lookup
							if spi_ext_out = (spi_ext_out'range => '1') then
								-- subsequent received data will be redirected to the FIFO
								spi_ext_fifo_data <= '1';
							elsif spi_ext_out(15 downto 9) = "1000000" then
								-- subsequent data is for FIR coefficients
								spi_ext_fir_data <= '1';
								mod_reload_FIR <= '1';
							else
								-- subsequent data is for the I/Q lookup
								spi_ext_I_Q_address <= spi_ext_out(8 downto 0);
							end if;
						else
							-- not the first word
							if spi_ext_fifo_data = '1' then
								mod_src_write <= '1';
							elsif spi_ext_fir_data = '1' then
								mod_write_FIR <= '1';
							else
								mod_write_I_Q <= "1";
							end if;
						end if;
					end if;
				end if;
				if spi_ext_fifo_data = '1' then
					spi_ext_in <= "00" & mod_src_fifo_level;
				else
					spi_ext_in <= "0000000" & spi_ext_I_Q_address;
				end if;
	-----------------------------------------------
	-- STOP: External SPI handling
	-----------------------------------------------
	-----------------------------------------------
	-- START: Internal SPI handling
	-----------------------------------------------		
				if SPI_INT_CS = '1' then
					first <= '1';
					mem_address <= "000000000000000";
				else
					if spi_int_complete = '1' then
						if first = '1' then
							write_not_read <= spi_int_out(15);
							mem_address <= spi_int_out(14 downto 0);
							first <= '0';
						else
							if write_not_read = '1' then
								case mem_address is
									when "000000000000000" =>
										-- write to GPIO register
										SW1_CTL <= spi_int_out(2 downto 0);
										SW2_CTL <= spi_int_out(6 downto 4);
										MOD_EN <= spi_int_out(7);
										SW1_CTL <= spi_int_out(2 downto 0);
										--LED <= spi_int_out(12 downto 8);
										EXT_PORT_VALUES(15 downto 12) <= spi_int_out(15 downto 12);
									-- Ext ADC range and LED control
									when "000000000000001" =>
										EXT_ADC_LED_I_ENABLE <= spi_int_out(14);
										EXT_ADC_LED_Q_ENABLE <= spi_int_out(15);
										EXT_ADC_MAX <= unsigned(spi_int_out(9 downto 0));
									-- modulation source phase increment
									when "000000000000100" =>
										mod_src_pinc <= mod_src_pinc(19 downto 16) & spi_int_out;
									-- modulation setting1
									when "000000000000101" =>
										mod_setting1 <= spi_int_out;
									-- modulation setting2
									when "000000000000110" =>
										mod_setting2 <= spi_int_out;
									-- modulation and source types
									when "000000000000111" =>
										mod_type <= spi_int_out(7 downto 0);
										mod_src_type <= spi_int_out(11 downto 8);
										mod_src_pinc <= spi_int_out(15 downto 12) & mod_src_pinc(15 downto 0);
									-- modulation setting3
									when "000000000001000" =>
										mod_setting3 <= spi_int_out;
									when others =>
								end case;
							end if;
							-- increment address
							mem_address <= std_logic_vector(unsigned(mem_address) + 1);
						end if;
					end if;
				end if;
				-- assign spi_in dependent on mem_address
		--			spi_in <= '0' & mem_address;
--				case mem_address is
--					when "000000000000000" =>
--						spi_int_in <= EXT_PORT_UPDATED & "00" & LED(4 downto 0) & MOD_EN & SW2_CTL(2 downto 0) & "0" & SW1_CTL(2 downto 0);
--	--				when "000000000000010" =>
--	--					spi_int_in <= "0000" & I_direct;
--	--				when "000000000000011" =>
--	--					spi_int_in <= "0000" & Q_direct;
--					when "000000000000100" =>
--						spi_int_in <= mod_src_pinc(15 downto 0);
--					when "000000000000101" =>
--						spi_int_in <= mod_setting1;
--					when "000000000000110" =>
--						spi_int_in <= mod_setting2;
--					when "000000000000111" =>
--						spi_int_in <= mod_src_pinc(19 downto 16) & mod_src_type & mod_type;
--					when "000000000001000" =>
--						spi_int_in <= mod_setting3;
--					when others => spi_int_in <= (others => '0');
--				end case;
	-----------------------------------------------
	-- STOP: Internal SPI handling
	-----------------------------------------------

			end if;
		end if;
	end process;

end Behavioral;

