----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    17:42:54 10/20/2019 
-- Design Name: 
-- Module Name:    PCA9555 - Behavioral 
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

entity PCA9555 is
	 GENERIC(
			CLK_FREQ : integer;
			ADDRESS : std_logic_vector(6 downto 0)
	 );
    Port ( CLK : in  STD_LOGIC;
           RESET : in  STD_LOGIC;
           SCL : out  STD_LOGIC;
           SDA : inout  STD_LOGIC;
           GPO : in  STD_LOGIC_VECTOR (15 downto 0);
           UPDATED : out  STD_LOGIC);
end PCA9555;

architecture Behavioral of PCA9555 is
	signal configured : std_logic;
	signal gpo_state : std_logic_vector(15 downto 0);
	signal write_dest : std_logic_vector(0 downto 0);
	signal write_value : std_logic_vector(15 downto 0);
	signal state : integer range 0 to 15;
	signal busy : std_logic;
	
	signal i2c_busy : std_logic;
	signal i2c_start : std_logic;
	signal i2c_stop : std_logic;
	signal i2c_write : std_logic;
	signal i2c_ack : std_logic;
	signal i2c_data : std_logic_vector(7 downto 0);
	
	COMPONENT i2c
	generic (
		CLK_FREQ : integer;
		I2C_FREQ : integer
	);
	PORT(
		CLK : IN std_logic;
		RESET : IN std_logic;
		START : IN std_logic;
		STOP : IN std_logic;
		DATA : IN std_logic_vector(7 downto 0);
		WR : IN std_logic;    
		SDA : INOUT std_logic;      
		BUSY : OUT std_logic;
		ACK : OUT std_logic;
		SCL : OUT std_logic
		);
	END COMPONENT;
begin

	I2C_Master: i2c
	GENERIC MAP(
		CLK_FREQ => CLK_FREQ,
		I2C_FREQ => 100000
	)
	PORT MAP(
		CLK => CLK,
		RESET => RESET,
		START => i2c_start,
		STOP => i2c_stop,
		DATA => i2c_data,
		WR => i2c_write,
		BUSY => i2c_busy,
		ACK => i2c_ack,
		SDA => SDA,
		SCL => SCL
	);

	process(CLK, RESET)
	begin
		if(rising_edge(CLK)) then
			if(RESET = '1') then
				configured <= '0';
				gpo_state <= (others=>'1');--PCA9555 output are set to high after reset
				state <= 0;
				busy <= '0';
				UPDATED <= '0';
				i2c_start <= '0';
				i2c_stop <= '0';
				i2c_write <= '0';
			else
				if(busy = '1') then
					if(i2c_busy = '1') then
						i2c_start <= '0';
						i2c_stop <= '0';
						i2c_write <= '0';
					elsif(i2c_start ='0' and i2c_stop = '0' and i2c_write = '0') then
						state <= state + 1;
						case(state) is
							when 0 =>
								i2c_start <= '1';
							when 1 =>
								i2c_data <= ADDRESS & '0';
								i2c_write <= '1';
							when 2 =>
								i2c_data <= "00000" & write_dest & "10";
								i2c_write <= '1';
							when 3 =>
								i2c_data <= write_value(7 downto 0);
								i2c_write <= '1';
							when 4 =>
								i2c_data <= write_value(15 downto 8);
								i2c_write <= '1';
							when 5 =>
								i2c_stop <= '1';
							when others =>
								busy <= '0';
								if(write_dest = "1") then
									configured <= '1';
								elsif(write_dest = "0") then
									UPDATED <= '1';
									gpo_state <= GPO;
								end if;
						end case;
						if(state > 1 and i2c_ack = '0') then
							-- acknowledge failure
							configured <= '0';
							busy <= '0';
							UPDATED <= '0';
							i2c_write <= '0';
							i2c_stop <= '1';
						end if;
					end if;
				else
					if(configured = '0') then
						-- configure pins 2-15 as outputs, pin 0-1 as input
						write_dest <= "1";
						write_value <= "0000000000000011";
						busy <= '1';
						state <= 0;
					elsif (gpo_state /= GPO) then
						-- update outputs
						UPDATED <= '0';
						write_dest <= "0";
						write_value <= GPO;
						busy <= '1';
						state <= 0;
					end if;
				end if;
			end if;
		end if;
	end process;

end Behavioral;

