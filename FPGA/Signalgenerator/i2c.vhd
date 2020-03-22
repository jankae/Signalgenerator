----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    16:22:47 10/20/2019 
-- Design Name: 
-- Module Name:    i2c - Behavioral 
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

entity i2c is
	generic (
		CLK_FREQ : integer;
		I2C_FREQ : integer
	);
	Port (
		CLK : in  STD_LOGIC;
		RESET : in  STD_LOGIC;
		START : in  STD_LOGIC;
		STOP : in  STD_LOGIC;
		DATA : in  STD_LOGIC_VECTOR (7 downto 0);
		WR : in  STD_LOGIC;
		BUSY : out  STD_LOGIC;
		ACK : out  STD_LOGIC;
		SDA : inout STD_LOGIC;
		SCL : out STD_LOGIC
	);
end i2c;

architecture Behavioral of i2c is
	signal clk_cnt : integer range 0 to (CLK_FREQ/I2C_FREQ)/4-1;
	signal data_latched : std_logic_vector(8 downto 0); -- also contains dummy bit for ack
	signal state : integer range 0 to 36;
	signal i2c_busy : std_logic;
	signal generate_start : std_logic;
	signal generate_stop : std_logic;
	signal sda_out : std_logic;
	signal sda_in : std_logic;
begin

	BUSY <= i2c_busy;
	SDA <= '0' when sda_out = '0' else 'Z';
	sda_in <= SDA;

	process(CLK, RESET)
	begin
		if(rising_edge(CLK)) then
			if(RESET = '1') then
				-- TODO reset internal states
				clk_cnt <= 0;
				state <= 0;
				i2c_busy <= '0';
				SCL <= 'Z';
				sda_out <= '1';
				ACK <= '0';
				generate_start <= '0';
				generate_stop <= '0';
				data_latched <= (others => '0');
			elsif(i2c_busy = '1') then
				if (clk_cnt = 0) then
					clk_cnt <= (CLK_FREQ/I2C_FREQ)/4-1;
					state <= state+1;
					-- next step in I2C state machine
					if(generate_start = '1') then
						case(state) is
							when 0 =>
								sda_out <= '1';
							when 1 =>
								SCL <= 'Z';
							when 2 =>
								sda_out <= '0';
							when 3 =>
								SCL <= '0';
							when others =>
								i2c_busy <= '0';
								generate_start <= '0';
								state <= 0;
								clk_cnt <= 0;
						end case;
					elsif(generate_stop = '1') then
						case(state) is
							when 0 =>
								sda_out <= '0';
							when 1 =>
								SCL <= 'Z';
							when 2 =>
								sda_out <= '1';
							when others =>
								i2c_busy <= '0';
								generate_stop <= '0';
								state <= 0;
								clk_cnt <= 0;
						end case;
					else
						--neither stop nor start, must be sending data
						case(state mod 4) is
							when 0 =>
								SCL <= '0';
								if(data_latched(8) = '0') then
									sda_out <= '0';
								else
									sda_out <= '1';
								end if;
								data_latched <= data_latched(7 downto 0) & '1';
							when 1|2 =>
								SCL <= 'Z';
							when 3 =>
								SCL <= '0';
						end case;
						if(state = 34) then
							if(sda_in = '1') then
								ACK <= '0';
							else
								ACK <= '1';
							end if;
						elsif(state = 36) then
							i2c_busy <= '0';
							state <= 0;
							clk_cnt <= 0;
						end if;
					end if;
				else
					clk_cnt <= clk_cnt-1;					
				end if;
			else
				state <= 0;
				-- not busy, check for new work
				if(START = '1') then
					generate_start <= '1';
					i2c_busy <= '1';
				elsif(STOP = '1') then
					generate_stop <= '1';
					i2c_busy <= '1';
				elsif(WR = '1') then
					data_latched <= DATA & '1';
					i2c_busy <= '1';
				end if;
			end if;
		end if;
	end process;

end Behavioral;

