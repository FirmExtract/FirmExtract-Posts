module fault_injection(BUTTON, SW, LEDG, GPIO0_D, CLOCK_50);
	input [1:0]BUTTON;
	input [9:0]SW;
	input CLOCK_50;
	
	output [1:0]GPIO0_D;
	output [1:0]LEDG;
	
	reg [9:0]count;
	reg [50:0]counter;
	reg [1:0]state;
	reg gpio_reg;
	reg led_reg;

assign GPIO0_D[0] = gpio_reg;
assign LEDG[0] = led_reg;

always @ (posedge CLOCK_50) begin
	if (state == 0) begin 
		if(BUTTON[0] == 0) begin
			count <= SW;
			state <= 1;
		end
	end
	else if(state == 1) begin
		led_reg <= 1;
		gpio_reg <= 0;
		counter <= counter + 1;
		if(counter==count*100) begin
			gpio_reg <= 1;
			counter <= 0;
			state <= 2;
			led_reg <= 0;
			count <= 0;
		end
	end
	else if(state == 2) begin
		counter <= counter + 1;
		led_reg <= 0;
		if(counter == 10000000) begin
			counter <= 0;
			state <= 0;
		end
	end 
	else begin
		state <= 0;
		gpio_reg <= 1;
		led_reg <= 0;
	end
end

endmodule