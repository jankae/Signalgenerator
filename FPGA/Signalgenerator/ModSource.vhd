----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    12:28:47 07/05/2019 
-- Design Name: 
-- Module Name:    ModSource - Behavioral 
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

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity ModSource is
	GENERIC ( STREAM_DEPTH : integer);
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
			  -- Modulation source type
			  -- 0000: Source disabled
			  -- 0001: Fixed value (uses 12LSBs of PINC as output value)
			  -- 0010: Sine
			  -- 0011: Ramp Up
			  -- 0100: Ramp down
			  -- 0101: Triangle
			  -- 0110: Square
			  -- 0111: PRBS (PINC determines update rate)
			  -- 1000: FIFO stream (PINC determines update rate)
			  -- 1001-1111: reserved
           SRCTYPE : in  STD_LOGIC_VECTOR (3 downto 0);
           PINC : in  STD_LOGIC_VECTOR (15 downto 0);
           RESULT : out  STD_LOGIC_VECTOR (11 downto 0);
			  NEW_SAMPLE : out STD_LOGIC;
			  SPI_SCK : in STD_LOGIC;
			  SPI_CS : in STD_LOGIC;
			  SPI_MOSI : in STD_LOGIC;
			  SPI_MISO : out STD_LOGIC
			  );
end ModSource;

architecture Behavioral of ModSource is
	COMPONENT PRBS
	generic (W : integer);
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;          
		RESULT : OUT std_logic_vector(W-1 downto 0)
		);
	END COMPONENT;
	COMPONENT SourceDDS
	PORT (
		ce : IN STD_LOGIC;
		clk : IN STD_LOGIC;
		sclr : IN STD_LOGIC;
		pinc_in : IN STD_LOGIC_VECTOR(26 DOWNTO 0);
		sine : OUT STD_LOGIC_VECTOR(11 DOWNTO 0);
		phase_out : OUT STD_LOGIC_VECTOR(26 DOWNTO 0)
	);
	END COMPONENT;
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
	COMPONENT FIFO
		generic (
		Datawidth : integer;
		Addresswidth : integer
	);
	PORT(
		CLK : IN  std_logic;
		DIN : IN  std_logic_vector(Datawidth-1 downto 0);
		DOUT : OUT  std_logic_vector(Datawidth-1 downto 0);
		WR : IN  std_logic;
		RD : IN  std_logic;
		CLEAR : IN std_logic;
		LEVEL : OUT  std_logic_vector(Addresswidth-1 downto 0)
	);
	END COMPONENT;
	signal prbs_value : std_logic_vector(11 downto 0);
	signal sine_value : std_logic_vector(11 downto 0);
	signal phase_value : std_logic_vector(26 downto 0);
	signal phase_inc : std_logic_vector(26 downto 0);
	signal dds_enable : std_logic;
	signal prbs_reset : std_logic;
	signal updated : std_logic;
	
	signal spi_out : std_logic_vector(7 downto 0);
	signal spi_in : std_logic_vector(7 downto 0);
	signal spi_complete : std_logic;

	signal fifo_clear : std_logic;
	signal fifo_out : std_logic_vector(7 downto 0);
	signal fifo_read : std_logic;
	signal fifo_level : std_logic_vector(STREAM_DEPTH-1 downto 0);
begin

	PRBS_Generator: PRBS
	GENERIC MAP(W => 12)
	PORT MAP(
		CLK => CLK,
		RESET => prbs_reset,
		RESULT => prbs_value
	);
	
	SINE_Generator : SourceDDS
	PORT MAP (
		ce => dds_enable,
		clk => CLK,
		sclr => RESET,
		pinc_in => phase_inc,
		sine => sine_value,
		phase_out => phase_value
	);
	
	SPI_interface : spi_slave 
	GENERIC MAP(W => 8)
	PORT MAP(
		SPI_CLK => SPI_SCK,
		MISO => SPI_MISO,
		MOSI => SPI_MOSI,
		CS => SPI_CS,
		BUF_OUT => spi_out,
		BUF_IN => spi_in,
		CLK => CLK,
		COMPLETE => spi_complete
	);
	
	ModStream: FIFO 
	GENERIC MAP (
		Addresswidth => STREAM_DEPTH,
		Datawidth => 8
	)
	PORT MAP (
		CLK => CLK,
		DIN => spi_out,
		DOUT => fifo_out,
		WR => spi_complete,
		RD => fifo_read,
		CLEAR => fifo_clear,
		LEVEL => fifo_level
	);
	
	spi_in <= fifo_level(STREAM_DEPTH-1 downto STREAM_DEPTH-8);
	
	fifo_clear <= '0' when SRCTYPE = "1000" else '1';
	
	phase_inc <= "00000000000" & PINC;
	
	dds_enable <= '0' when SRCTYPE = "0000" or SRCTYPE = "0001" else '1';
	prbs_reset <= '0' when SRCTYPE = "0111" else '1';

	process(CLK, RESET)
	begin
		if(RESET = '1') then
			RESULT <= (others=>'0');
			updated <= '0';
			NEW_SAMPLE <= '0';
		elsif(rising_edge(CLK)) then
			case SRCTYPE is
				-- Source disabled
				when "0000" =>
					RESULT <= (others=>'0');
					NEW_SAMPLE <= '0';
				-- Fixed value
				when "0001" =>
					RESULT <= PINC(11 downto 0);
					NEW_SAMPLE <= '1';
				-- Sine, invert MSB to convert from 2's complement to binary
				when "0010" =>
					RESULT <= not sine_value(11) & sine_value(10 downto 0);
					NEW_SAMPLE <= '1';
				-- Ramp up, use upper bits of phase value
				when "0011" =>
					RESULT <= phase_value(26 downto 15);
					NEW_SAMPLE <= '1';
				-- Ramp down, use inverted upper bits of phase value
				when "0100" =>
					RESULT <= not phase_value(26 downto 15);
					NEW_SAMPLE <= '1';
				-- Triangle, use upper bits of phase value, invert if MSB set
				when "0101" =>
					NEW_SAMPLE <= '1';
					if(phase_value(26)='1') then
						RESULT <= not phase_value(25 downto 14);
					else
						RESULT <= phase_value(25 downto 14);
					end if;
				-- Square, set all bits if MSB of phase is set
				when "0110" =>
					NEW_SAMPLE <= '1';
					if(phase_value(26)='1') then
						RESULT <= (others => '1');
					else
						RESULT <= (others => '0');
					end if;
				-- PRBS
				when "0111" =>
					if(phase_value(26)='1') then
						if(updated = '0') then
							RESULT <= prbs_value;
							updated <= '1';
							NEW_SAMPLE <= '0';
						else
							NEW_SAMPLE <= '1';
						end if;
					else
						NEW_SAMPLE <= '0';
						updated <= '0';
					end if;
				-- FIFO stream
				when "1000" =>
					RESULT <= fifo_out & "0000";
					if(phase_value(26)='1') then
						if(updated = '0') then
							fifo_read <= '1';
							updated <= '1';
							NEW_SAMPLE <= '0';
						else
							NEW_SAMPLE <= '1';
							fifo_read <= '0';
						end if;
					else
						NEW_SAMPLE <= '0';
						fifo_read <= '0';
						updated <= '0';
					end if;
				-- invalid setting
				when others =>
					RESULT <= (others => '0');
			end case;
		end if;
	end process;

end Behavioral;

