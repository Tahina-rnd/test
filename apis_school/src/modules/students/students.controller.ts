import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
  UseGuards,
} from '@nestjs/common';
import { StudentsService } from './students.service';
import { CreateStudentDto } from './dto/create-student.dto';
import { UpdateStudentDto } from './dto/update-student.dto';
import { JwtAuthGuard } from '@common/guards/jwt-auth.guard';
import { RolesGuard } from '@common/guards/roles.guard';
import { Roles } from '@decorators/roles.decorator';
import { UserRole } from '@enums/user.enum';

@Controller('students')
@UseGuards(JwtAuthGuard, RolesGuard)
export class StudentsController {
  constructor(private readonly studentsService: StudentsService) {}

  /**
   * Create a new student.
   *
   * @param createStudentDto - The student data to create.
   *
   * @returns The created student.
   */
  @Post()
  @Roles(UserRole.ADMIN)
  create(@Body() createStudentDto: CreateStudentDto) {
    return this.studentsService.create(createStudentDto);
  }

  /**
   * Get all students.
   *
   * @returns An array of students.
   */
  @Get()
  @Roles(UserRole.ADMIN, UserRole.STUDENT)
  findAll() {
    return this.studentsService.findAll();
  }

  /**
   * Get a student by id.
   *
   * @param id - The id of the student to retrieve.
   *
   * @returns The student.
   */
  @Get(':id')
  @Roles(UserRole.ADMIN, UserRole.STUDENT)
  findOne(@Param('id') id: string) {
    return this.studentsService.findOne(id);
  }

  /**
   * Get a student by user id.
   *
   * @param userId - The user id of the student to retrieve.
   *
   * @returns The student.
   */
  @Get('user/:userId')
  @Roles(UserRole.ADMIN, UserRole.STUDENT)
  findByUserId(@Param('userId') userId: string) {
    return this.studentsService.findByUserId(userId);
  }

  /**
   * Update a student.
   *
   * @param id - The id of the student to update.
   * @param updateStudentDto - The student data to update.
   *
   * @returns The updated student.
   */
  @Patch(':id')
  @Roles(UserRole.ADMIN)
  update(@Param('id') id: string, @Body() updateStudentDto: UpdateStudentDto) {
    return this.studentsService.update(id, updateStudentDto);
  }

  /**
   * Delete a student.
   *
   * @param id - The id of the student to delete.
   */
  @Delete(':id')
  @Roles(UserRole.ADMIN)
  remove(@Param('id') id: string) {
    return this.studentsService.remove(id);
  }
}
