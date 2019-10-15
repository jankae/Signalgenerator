----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    15:28:40 07/13/2019 
-- Design Name: 
-- Module Name:    FIFO - Behavioral 
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
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity FIFO is
	generic (
		Datawidth : integer;
		Addresswidth : integer
	);
	Port ( CLK : in  STD_LOGIC;
           DIN : in  STD_LOGIC_VECTOR (Datawidth-1 downto 0);
           DOUT : out  STD_LOGIC_VECTOR (Datawidth-1 downto 0);
           WR : in  STD_LOGIC;
           RD : in  STD_LOGIC;
			  CLEAR : in STD_LOGIC;
           LEVEL : out  STD_LOGIC_VECTOR (Addresswidth-1 downto 0));
end FIFO;

architecture Behavioral of FIFO is

signal wrcnt : unsigned (Addresswidth-1 downto 0) := (others => '0');
signal rdcnt : unsigned (Addresswidth-1 downto 0) := (others => '0');
type mem is array(0 to (2**Addresswidth)-1) of unsigned(Datawidth-1 downto 0);
signal memory : mem;   
signal full  : std_logic;
signal empty : std_logic;

begin
	process(CLK)
	begin
	if(rising_edge(CLK)) then
		if (CLEAR = '1') then
			wrcnt <= (others => '0');
			rdcnt <= (others => '0');
			full <= '0';
			empty <= '1';
			LEVEL <= std_logic_vector(to_signed(-1,LEVEL'length));
		else
			if (Wr='1' and full='0') then
				memory(to_integer(wrcnt)) <= unsigned(Din);
				wrcnt <= wrcnt+1;
				if (rdcnt = wrcnt) then
					full <= '1';
				end if;
				empty <= '0';
			end if;
			if (Rd='1' and empty='0') then
				Dout <= std_logic_vector(memory(to_integer(rdcnt)));
				rdcnt <= rdcnt+1;
				full <= '0';
				if (rdcnt + 1 = wrcnt) then
					empty <= '1';
				end if;
			end if;
			LEVEL <= std_logic_vector(rdcnt - wrcnt - 1);
		end if;
	end if;
	end process;
end Behavioral;

