library IEEE;
	use IEEE.std_logic_1164.all;
	use IEEE.std_logic_arith.all;
	use IEEE.std_logic_unsigned.all;

library PROC_COMMON_V3_00_A;
	use PROC_COMMON_V3_00_A.proc_common_pkg.all;

entity user_logic is
	generic (
		C_NUM_REG 		: integer := 8;
		C_SLV_DWIDTH 	: integer := 32
	);
	port (
		INTX_MSI_Grant 		: in  std_logic;
		INTX_MSI_Request 	: out std_logic;

		MSI_Enable			: in  std_logic;
		MSI_Width			: in  std_logic_vector(2 downto 0);
		MSI_Num				: out std_logic_vector(4 downto 0);

		Bus2IP_Clk 			: in  std_logic;
		Bus2IP_Resetn 		: in  std_logic;
		Bus2IP_Data 		: in  std_logic_vector(C_SLV_DWIDTH - 1 downto 0);
		Bus2IP_BE 			: in  std_logic_vector(C_SLV_DWIDTH/8 - 1 downto 0);
		Bus2IP_RdCE 		: in  std_logic_vector(C_NUM_REG - 1 downto 0);
		Bus2IP_WrCE 		: in  std_logic_vector(C_NUM_REG - 1 downto 0);
		IP2Bus_Data 		: out std_logic_vector(C_SLV_DWIDTH - 1 downto 0);
		IP2Bus_RdAck 		: out std_logic;
		IP2Bus_WrAck 		: out std_logic;
		IP2Bus_Error 		: out std_logic
	);

	attribute MAX_FANOUT	: string;
	attribute SIGIS 		: string;

	attribute SIGIS of Bus2IP_Clk 		: signal is "CLK";
	attribute SIGIS of Bus2IP_Resetn 	: signal is "RST";

end entity;

architecture rtl of user_logic is

	signal slv_reg0 			: std_logic_vector(C_SLV_DWIDTH - 1 downto 0);
	signal slv_reg1 			: std_logic_vector(C_SLV_DWIDTH - 1 downto 0);
	signal slv_reg2 			: std_logic_vector(C_SLV_DWIDTH - 1 downto 0);
	signal slv_reg3 			: std_logic_vector(C_SLV_DWIDTH - 1 downto 0);
	signal slv_reg4 			: std_logic_vector(C_SLV_DWIDTH - 1 downto 0);
	signal slv_reg5 			: std_logic_vector(C_SLV_DWIDTH - 1 downto 0);
	signal slv_reg6 			: std_logic_vector(C_SLV_DWIDTH - 1 downto 0);
	signal slv_reg7 			: std_logic_vector(C_SLV_DWIDTH - 1 downto 0);
	signal slv_reg_write_sel 	: std_logic_vector(7 downto 0);
	signal slv_reg_read_sel 	: std_logic_vector(7 downto 0);
	signal slv_ip2bus_data 		: std_logic_vector(C_SLV_DWIDTH - 1 downto 0);
	signal slv_read_ack 		: std_logic;
	signal slv_write_ack 		: std_logic;

	signal msi_irq				: std_logic;
	signal resync 				: std_logic_vector(1 to 2);

begin

	slv_reg_write_sel	<= Bus2IP_WrCE(7 downto 0);
	slv_reg_read_sel 	<= Bus2IP_RdCE(7 downto 0);
	slv_write_ack 		<= Bus2IP_WrCE(0) or Bus2IP_WrCE(1) or Bus2IP_WrCE(2) or Bus2IP_WrCE(3) or Bus2IP_WrCE(4) or Bus2IP_WrCE(5) or Bus2IP_WrCE(6) or Bus2IP_WrCE(7);
	slv_read_ack 		<= Bus2IP_RdCE(0) or Bus2IP_RdCE(1) or Bus2IP_RdCE(2) or Bus2IP_RdCE(3) or Bus2IP_RdCE(4) or Bus2IP_RdCE(5) or Bus2IP_RdCE(6) or Bus2IP_RdCE(7);

	process(Bus2IP_Clk)
	begin
		if rising_edge(Bus2IP_Clk) then
			if (Bus2IP_Resetn = '0') then
				slv_reg0 <= (others => '0');
				slv_reg1 <= (others => '0');
				slv_reg2 <= (others => '0');
				slv_reg3 <= (others => '0');
				slv_reg4 <= (others => '0');
				slv_reg5 <= (others => '0');
				slv_reg6 <= (others => '0');
				slv_reg7 <= (others => '0');
			else
				case slv_reg_write_sel is
					when "10000000" =>
						for byte_index in 0 to (C_SLV_DWIDTH/8) - 1 loop
							if (Bus2IP_BE(byte_index) = '1') then
								slv_reg0(byte_index * 8 + 7 downto byte_index * 8) <= Bus2IP_Data(byte_index * 8 + 7 downto byte_index * 8);
							end if;
						end loop;
					when "01000000" =>
						for byte_index in 0 to (C_SLV_DWIDTH/8) - 1 loop
							if (Bus2IP_BE(byte_index) = '1') then
								slv_reg1(byte_index * 8 + 7 downto byte_index * 8) <= Bus2IP_Data(byte_index * 8 + 7 downto byte_index * 8);
							end if;
						end loop;
					when "00100000" =>
						for byte_index in 0 to (C_SLV_DWIDTH/8) - 1 loop
							if (Bus2IP_BE(byte_index) = '1') then
								slv_reg2(byte_index * 8 + 7 downto byte_index * 8) <= Bus2IP_Data(byte_index * 8 + 7 downto byte_index * 8);
							end if;
						end loop;
					when "00010000" =>
						for byte_index in 0 to (C_SLV_DWIDTH/8) - 1 loop
							if (Bus2IP_BE(byte_index) = '1') then
								slv_reg3(byte_index * 8 + 7 downto byte_index * 8) <= Bus2IP_Data(byte_index * 8 + 7 downto byte_index * 8);
							end if;
						end loop;
					when "00001000" =>
						for byte_index in 0 to (C_SLV_DWIDTH/8) - 1 loop
							if (Bus2IP_BE(byte_index) = '1') then
								slv_reg4(byte_index * 8 + 7 downto byte_index * 8) <= Bus2IP_Data(byte_index * 8 + 7 downto byte_index * 8);
							end if;
						end loop;
					when "00000100" =>
						for byte_index in 0 to (C_SLV_DWIDTH/8) - 1 loop
							if (Bus2IP_BE(byte_index) = '1') then
								slv_reg5(byte_index * 8 + 7 downto byte_index * 8) <= Bus2IP_Data(byte_index * 8 + 7 downto byte_index * 8);
							end if;
						end loop;
					when "00000010" =>
						for byte_index in 0 to (C_SLV_DWIDTH/8) - 1 loop
							if (Bus2IP_BE(byte_index) = '1') then
								slv_reg6(byte_index * 8 + 7 downto byte_index * 8) <= Bus2IP_Data(byte_index * 8 + 7 downto byte_index * 8);
							end if;
						end loop;
					when "00000001" =>
						for byte_index in 0 to (C_SLV_DWIDTH/8) - 1 loop
							if (Bus2IP_BE(byte_index) = '1') then
								slv_reg7(byte_index * 8 + 7 downto byte_index * 8) <= Bus2IP_Data(byte_index * 8 + 7 downto byte_index * 8);
							end if;
						end loop;
					when others => null;
				end case;
			end if;
		end if;
	end process;

	process (slv_reg_read_sel, slv_reg0, slv_reg1, slv_reg2, slv_reg3, slv_reg4, slv_reg5, slv_reg6, slv_reg7)
	begin
		case slv_reg_read_sel is
			when "10000000" => slv_ip2bus_data <= x"000000" & "000" & MSI_Width & INTX_MSI_Grant & MSI_Enable;
			when "01000000" => slv_ip2bus_data <= slv_reg1;
			when "00100000" => slv_ip2bus_data <= slv_reg2;
			when "00010000" => slv_ip2bus_data <= slv_reg3;
			when "00001000" => slv_ip2bus_data <= slv_reg4;
			when "00000100" => slv_ip2bus_data <= slv_reg5;
			when "00000010" => slv_ip2bus_data <= slv_reg6;
			when "00000001" => slv_ip2bus_data <= slv_reg7;
			when others 	=> slv_ip2bus_data <= (others => '0');
		end case;
	end process;

	IP2Bus_Data 	<= slv_ip2bus_data when slv_read_ack = '1' else (others => '0');
	IP2Bus_WrAck 	<= slv_write_ack;
	IP2Bus_RdAck 	<= slv_read_ack;
	IP2Bus_Error 	<= '0';


	-- Status register
	-- slv_reg0(0) 			<= MSI_Enable;
	-- slv_reg0(1) 			<= INTX_MSI_Grant;
	-- slv_reg0(4 downto 2) 	<= MSI_Width;

	msi_irq <= slv_reg1(0) when MSI_Enable = '1' else '0';
	
	MSI_Num <= slv_reg1(5 downto 1);


    process(Bus2IP_Clk)
    begin
        if rising_edge(Bus2IP_Clk) then
			resync				<= msi_irq & resync(1);
			INTX_MSI_Request	<= resync(1) and not(resync(2));
        end if;
    end process;

end architecture;