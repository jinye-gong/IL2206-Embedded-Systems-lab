package Buffer is

   protected type Watchdog_PO is
      procedure Feed;
      function Is_Fed return Boolean;
      procedure Reset;
   private
      Fed : Boolean := False;
   end Watchdog_PO;

end Buffer;

