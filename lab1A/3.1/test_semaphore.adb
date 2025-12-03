with Ada.Text_IO; use Ada.Text_IO;
with Semaphores;  use Semaphores;

procedure Test_Semaphore is
	   S : CountingSemaphore (3,2);

		    task T1;
				task T2;
				task T3;
				task T4;
				
				
				task body T1 is
					begin
						 S.Wait;
						 Put_Line("Task 1 entered");
						 delay 2.0;
						 Put_Line("Task 1 leaving");
					   S.Signal;
					   end T1;

			  task body T2 is
					begin
			      S.Wait;
						Put_Line("Task 2 entered");		
						delay 2.0;
			      Put_Line("Task 2 leaving");		
						S.Signal;
		 	   end T2;

				 task body T3 is
			    begin
						S.Wait;
			      Put_Line("Task 3 entered");
				    delay 2.0;
					  Put_Line("Task 3 leaving");
					  S.Signal;
		 		   end T3;

				 task body T4 is
					 begin
						S.Wait;
						Put_Line("Task 4 entered");
						delay 2.0;
						Put_Line("Task 4 leaving");
						S.Signal;
					 end T4;
begin 															
	null;
end Test_Semaphore;

