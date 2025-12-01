set breakpoint pending on
set confirm off
file ./levic
break leviathan::Err::report
commands
	where
end
break leviathan::ToDoError::ToDoError
commands
	where
end
break leviathan::InternalError::InternalError
commands
	where
end

define t7
  set args p7_tests/$arg0.levi -o --
  run
end
