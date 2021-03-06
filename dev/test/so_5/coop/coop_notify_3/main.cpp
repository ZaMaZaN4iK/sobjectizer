/*
 * A test for stardard coop reg/dereg notificators.
 */

#include <iostream>
#include <sstream>

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/time_limited_execution.hpp>

struct msg_child_deregistered : public so_5::signal_t {};

class a_child_t : public so_5::agent_t
{
		typedef so_5::agent_t base_type_t;

	public :
		a_child_t(
			so_5::environment_t & env )
			:	base_type_t( env )
		{
		}
};

class a_test_t : public so_5::agent_t
{
	typedef so_5::agent_t base_type_t;

	public :
		a_test_t(
			so_5::environment_t & env )
			:	base_type_t( env )
			,	m_mbox( env.create_mbox() )
			,	m_cycle( 0 )
		{}

		void
		so_define_agent()
		{
			so_subscribe( m_mbox ).in( st_wait_registration )
					.event( &a_test_t::evt_coop_registered );
			so_subscribe( m_mbox ).in( st_wait_deregistration )
					.event( &a_test_t::evt_coop_deregistered );
		}

		void
		so_evt_start()
		{
			so_change_state( st_wait_registration );

			create_next_coop();
		}

		void
		evt_coop_registered(
			mhood_t< so_5::msg_coop_registered > evt )
		{
			std::cout << "registered: " << evt->m_coop << std::endl;

			so_change_state( st_wait_deregistration );

			so_environment().deregister_coop(
					evt->m_coop,
					so_5::dereg_reason::normal );
		}

		void
		evt_coop_deregistered(
			mhood_t< so_5::msg_coop_deregistered > evt )
		{
			std::cout << "deregistered: " << evt->m_coop << std::endl;

			if( 5 == m_cycle )
				so_environment().stop();
			else
			{
				++m_cycle;
				so_change_state( st_wait_registration );

				create_next_coop();
			}
		}

	private :
		const so_5::mbox_t m_mbox;

		int m_cycle;

		so_5::state_t st_wait_registration{ this };
		so_5::state_t st_wait_deregistration{ this };

		void
		create_next_coop()
		{
			auto child_coop = so_environment().make_coop(
					so_coop(),
					so_5::disp::active_obj::make_dispatcher(
							so_environment() ).binder() );

			child_coop->add_reg_notificator(
					so_5::make_coop_reg_notificator( m_mbox ) );
			child_coop->add_dereg_notificator(
					so_5::make_coop_dereg_notificator( m_mbox ) );

			child_coop->make_agent< a_child_t >();

			so_environment().register_coop( std::move( child_coop ) );
		}
};

int
main()
{
	run_with_time_limit( [] {
			so_5::launch(
				[]( so_5::environment_t & env )
				{
					env.register_agent_as_coop(
							env.make_agent< a_test_t >() );
				} );
		},
		10 );

	return 0;
}

